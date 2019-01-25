/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdint.h"
#include "USBSerial.h"

/* Begin by Yanminge 2019-01-25 */
#if SMOOTHIEWARE_FEATURE_ENABLE
#include "libs/Kernel.h"
#include "libs/SerialMessage.h"
#include "StreamOutputPool.h"
#endif /* SMOOTHIEWARE_FEATURE_ENABLE */
/* End by Yanminge 2019-01-25 */

int USBSerial::_putc(int c) {
    if (!terminal_connected)
        return 0;
    send((uint8_t *)&c, 1);
    return 1;
}

int USBSerial::_getc() {
    uint8_t c = 0;
    while (buf.isEmpty());
    buf.dequeue(&c);
    return c;
}

/* Begin by Yanminge 2019-01-25 */
#if SMOOTHIEWARE_FEATURE_ENABLE
int USBSerial::puts(const char *str) {
    if (!terminal_connected)
        return strlen(str);
    int i = 0;
	while(*str)
    {
      send((uint8_t *)str,1);
	  i++;
	  str++;
	}
	return i;
}

void USBSerial::rx_callback(void){
}

#endif /* SMOOTHIEWARE_FEATURE_ENABLE */
/* End by Yanminge 2019-01-25 */

bool USBSerial::writeBlock(uint8_t * buf, uint16_t size) {
    if(size > MAX_PACKET_SIZE_EPBULK) {
        return false;
    }
    if(!send(buf, size)) {
        return false;
    }
    return true;
}

bool USBSerial::EPBULK_OUT_callback() {
    uint8_t c[65];
    uint32_t size = 0;

    //we read the packet received and put it on the circular buffer
    readEP(c, &size);
    for (uint32_t i = 0; i < size; i++) {
/* Begin by Yanminge 2019-01-25 */
#if SMOOTHIEWARE_FEATURE_ENABLE
        // handle backspace and delete by deleting the last character in the buffer if there is one
        if(c[i] == 0x08 || c[i] == 0x7F) {
            if(!buf.isEmpty()) buf.pop();
            continue;
        }

        if(c[i] == 'X' - 'A' + 1) { // ^X
            //THEKERNEL->set_feed_hold(false); // required to free stuff up
            halt_flag = true;
            continue;
        }

        if(c[i] == '?') { // ?
            query_flag = true;
            continue;
        }

        if(THEKERNEL->is_grbl_mode() || THEKERNEL->is_feed_hold_enabled()) {
            if(c[i] == '!') { // safe pause
                THEKERNEL->set_feed_hold(true);
                continue;
            }

            if(c[i] == '~') { // safe resume
                THEKERNEL->set_feed_hold(false);
                continue;
            }
        }
        buf.queue(c[i]);
		if (c[i] == '\n' || c[i] == '\r') {
            nl_in_rx++;
        }
#else /* SMOOTHIEWARE_FEATURE_ENABLE */
        buf.queue(c[i]);
#endif /* SMOOTHIEWARE_FEATURE_ENABLE */
/* End by Yanminge 2019-01-25 */
    }

    //call a potential handlenr
    if (rx)
        rx.call();

    return true;
}

uint8_t USBSerial::available() {
    return buf.available();
}

bool USBSerial::connected() {
    return terminal_connected;
}

/* Begin by Yanminge 2019-01-25 */
#if SMOOTHIEWARE_FEATURE_ENABLE
void USBSerial::on_module_loaded()
{
    this->register_for_event(ON_MAIN_LOOP);
    this->register_for_event(ON_IDLE);
	this->attach(callback(this, &USBSerial::rx_callback));
    nl_in_rx = 0;
    attached = false;
    halt_flag = false;
    query_flag = false;
}

void USBSerial::on_idle(void *argument)
{
    if(halt_flag) {
        halt_flag = false;
        THEKERNEL->call_event(ON_HALT, nullptr);
        if(THEKERNEL->is_grbl_mode()) {
            puts("ALARM: Abort during cycle\r\n");
        } else {
            puts("HALTED, M999 or $X to exit HALT state\r\n");
        }
        nl_in_rx = 0;
    }

    if(query_flag) {
        query_flag = false;
        puts(THEKERNEL->get_query_string().c_str());
    }

}

void USBSerial::on_main_loop(void *argument)
{
    // apparently some OSes don't assert DTR when a program opens the port
    if (connected() != attached)
	{
	    if(!connected())
        {
		    attached = false;
			THEKERNEL->streams->printf("USB disconnect!\r\n");
            THEKERNEL->streams->remove_stream(this);
		    nl_in_rx = 0;
        }
	    else
		{
		    attached = true;
            THEKERNEL->streams->append_stream(this);
            puts("Smoothie\r\nok\r\n");
	    }
    }
    // if we are in feed hold we do not process anything
    //if(THEKERNEL->get_feed_hold()) return;

    if (nl_in_rx) {
        string received;
        while (available()) {
            char c = _getc();
            if( c == '\n' || c == '\r') {
                struct SerialMessage message;
                message.message = received;
                message.stream = this;
                THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message );
                return;
            } else {
                received += c;
            }
        }
    }
}
#endif /* SMOOTHIEWARE_FEATURE_ENABLE */
/* End by Yanminge 2019-01-25 */
