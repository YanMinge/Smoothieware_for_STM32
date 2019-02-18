/*
 * Copyright (c) 2018, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "TCPSocket.h"
#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest.h"
#include "tcp_tests.h"

using namespace utest::v1;

namespace {
static const int SIGNAL_SIGIO = 0x1;
static const int SIGIO_TIMEOUT = 20000; //[ms]

static const int BUFF_SIZE = 1200;
static const int PKTS = 22;
static const int pkt_sizes[PKTS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, \
                                    100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, \
                                    1100, 1200
                                   };
TCPSocket sock;
Semaphore tx_sem(0, 1);
events::EventQueue *event_queue;
int bytes2recv;
int bytes2recv_total;

Timer tc_exec_time;
int time_allotted;
bool receive_error;
}

void tcpsocket_echotest_nonblock_receive();

static void _sigio_handler(osThreadId id)
{
    osSignalSet(id, SIGNAL_SIGIO);
    if (event_queue != NULL) {
        event_queue->call(tcpsocket_echotest_nonblock_receive);
    } else {
        TEST_FAIL_MESSAGE("_sigio_handler running when event_queue is NULL");
    }
}

void TCPSOCKET_ECHOTEST()
{
    if (tcpsocket_connect_to_echo_srv(sock) != NSAPI_ERROR_OK) {
        TEST_FAIL();
        return;
    }

    int recvd;
    int sent;
    int x = 0;
    for (int pkt_s = pkt_sizes[x]; x < PKTS; pkt_s = pkt_sizes[x++]) {
        fill_tx_buffer_ascii(tcp_global::tx_buffer, BUFF_SIZE);

        sent = sock.send(tcp_global::tx_buffer, pkt_s);
        if (sent < 0) {
            printf("[Round#%02d] network error %d\n", x, sent);
            TEST_FAIL();
            TEST_ASSERT_EQUAL(NSAPI_ERROR_OK, sock.close());
            return;
        }

        int bytes2recv = sent;
        while (bytes2recv) {
            recvd = sock.recv(&(tcp_global::rx_buffer[sent - bytes2recv]), bytes2recv);
            if (recvd < 0) {
                printf("[Round#%02d] network error %d\n", x, recvd);
                TEST_FAIL();
                TEST_ASSERT_EQUAL(NSAPI_ERROR_OK, sock.close());
                return;
            }
            bytes2recv -= recvd;
        }
        TEST_ASSERT_EQUAL(0, memcmp(tcp_global::tx_buffer, tcp_global::rx_buffer, sent));
    }
    TEST_ASSERT_EQUAL(NSAPI_ERROR_OK, sock.close());
}

void tcpsocket_echotest_nonblock_receive()
{
    int recvd = sock.recv(&(tcp_global::rx_buffer[bytes2recv_total - bytes2recv]), bytes2recv);
    if (recvd == NSAPI_ERROR_WOULD_BLOCK) {
        if (tc_exec_time.read() >= time_allotted) {
            receive_error = true;
        }
        return;
    } else if (recvd < 0) {
        receive_error = true;
    } else {
        bytes2recv -= recvd;
    }

    if (bytes2recv == 0) {
        TEST_ASSERT_EQUAL(0, memcmp(tcp_global::tx_buffer, tcp_global::rx_buffer, bytes2recv_total));

        static int round = 0;
        printf("[Recevr#%02d] bytes received: %d\n", round++, bytes2recv_total);
        tx_sem.release();
    } else if (receive_error || bytes2recv < 0) {
        TEST_FAIL();
        tx_sem.release();
    }
    // else - no error, not all bytes were received yet.
}

void TCPSOCKET_ECHOTEST_NONBLOCK()
{
#if MBED_CONF_NSAPI_SOCKET_STATS_ENABLE
    int j = 0;
    int count = fetch_stats();
    for (; j < count; j++) {
        TEST_ASSERT_EQUAL(SOCK_CLOSED, tcp_stats[j].state);
    }
#endif
    tc_exec_time.start();
    time_allotted = split2half_rmng_tcp_test_time(); // [s]

    EventQueue queue(2 * EVENTS_EVENT_SIZE);
    event_queue = &queue;

    tcpsocket_connect_to_echo_srv(sock);
    sock.set_blocking(false);
    sock.sigio(callback(_sigio_handler, ThisThread::get_id()));

    int bytes2send;
    int sent;
    int s_idx = 0;
    receive_error = false;
    unsigned char *stack_mem = (unsigned char *)malloc(tcp_global::TCP_OS_STACK_SIZE);
    TEST_ASSERT_NOT_NULL(stack_mem);
    Thread *receiver_thread = new Thread(osPriorityNormal,
                                         tcp_global::TCP_OS_STACK_SIZE,
                                         stack_mem,
                                         "receiver");

    TEST_ASSERT_EQUAL(osOK, receiver_thread->start(callback(&queue, &EventQueue::dispatch_forever)));

    for (int pkt_s = pkt_sizes[s_idx]; s_idx < PKTS; ++s_idx) {
        pkt_s = pkt_sizes[s_idx];
        bytes2recv = pkt_s;
        bytes2recv_total = pkt_s;

        fill_tx_buffer_ascii(tcp_global::tx_buffer, pkt_s);

        bytes2send = pkt_s;
        while (bytes2send > 0) {
            sent = sock.send(&(tcp_global::tx_buffer[pkt_s - bytes2send]), bytes2send);
            if (sent == NSAPI_ERROR_WOULD_BLOCK) {
                if (tc_exec_time.read() >= time_allotted ||
                        osSignalWait(SIGNAL_SIGIO, SIGIO_TIMEOUT).status == osEventTimeout) {
                    TEST_FAIL();
                    goto END;
                }
                continue;
            } else if (sent <= 0) {
                printf("[Sender#%02d] network error %d\n", s_idx, sent);
                TEST_FAIL();
                goto END;
            }
            bytes2send -= sent;
        }
        printf("[Sender#%02d] bytes sent: %d\n", s_idx, pkt_s);
#if MBED_CONF_NSAPI_SOCKET_STATS_ENABLE
        count = fetch_stats();
        for (j = 0; j < count; j++) {
            if ((tcp_stats[j].state == SOCK_OPEN) && (tcp_stats[j].proto == NSAPI_TCP)) {
                break;
            }
        }
        TEST_ASSERT_EQUAL(bytes2send, tcp_stats[j].sent_bytes);
#endif
        tx_sem.wait(split2half_rmng_tcp_test_time() * 1000); // *1000 to convert s->ms
        if (receive_error) {
            break;
        }
    }
END:
    sock.sigio(NULL);
    TEST_ASSERT_EQUAL(NSAPI_ERROR_OK, sock.close());
    receiver_thread->terminate();
    delete receiver_thread;
    receiver_thread = NULL;
    tc_exec_time.stop();
    free(stack_mem);
}
