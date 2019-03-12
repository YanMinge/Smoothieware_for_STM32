// Automatically generated configuration file.
// DO NOT EDIT, content will be overwritten.

#ifndef __MBED_CONFIG_DATA__
#define __MBED_CONFIG_DATA__

// Configuration parameters
#define MBED_CONF_LWIP_TCP_SERVER_MAX               4    // set by library:lwip
#define MBED_CONF_LWIP_ADDR_TIMEOUT                 5    // set by library:lwip
#define MBED_CONF_LWIP_IPV4_ENABLED                 0    // set by library:lwip
#define MBED_CONF_LWIP_TCP_SOCKET_MAX               4    // set by library:lwip
#define MBED_CONF_LWIP_IP_VER_PREF                  4    // set by library:lwip
#define MBED_CONF_LWIP_UDP_SOCKET_MAX               4    // set by library:lwip
#define MBED_CONF_LWIP_SOCKET_MAX                   4    // set by library:lwip
#define MBED_CONF_LWIP_IPV6_ENABLED                 0    // set by library:lwip

#define MBED_CONF_EVENTS_PRESENT                    0    // set by library:events
#define MBED_CONF_EVENTS_SHARED_EVENTSIZE           256  // set by library:events

#define MBED_CONF_FILESYSTEM_PRESENT                0    // set by library:filesystem

#define MBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT      1      // set by library:platform
#define MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE 115200 // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES   0      // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_BAUD_RATE          115200 // set by library:platform

#define MBED_CONF_PLATFORM_CTHUNK_COUNT_MAX         8

#define MBED_CONF_DATAFLASH_SPI_FREQ                40000000


// product Configuration
#define CLOCK_SOURCE                                USE_PLL_HSE_XTAL|USE_PLL_HSI
#define MBED_CONF_TARGET_LPUART_CLOCK_SOURCE        USE_LPUART_CLK_LSE|USE_LPUART_CLK_PCLK1 // set by target:FAMILY_STM32

#define MBED_CONF_TARGET_STDIO_UART_TX              PB_6//PA_9
#define MBED_CONF_TARGET_STDIO_UART_RX              PB_7//PA_10
#define STDIO_UART                                  UART_1

// Macros
#define UNITY_INCLUDE_CONFIG_H                           // defined by library:utest

// Define MBED_RTOS_WORDS_STACK_SIZE to override the size of the main thread's stack.
// It is in units of words so the actual stack will be MBED_RTOS_WORDS_STACK_SIZE * 4.
// It defaults to 512 words (4k) for GCC_ARM.
//#define MBED_RTOS_WORDS_STACK_SIZE                  128

// Set MBED_RTOS_SINGLE_THREAD macro if you are using mbed-os with a single threaded application so that it can
// reduce the task and timer counts accordingly to save RAM.
//#define MBED_RTOS_SINGLE_THREAD                     1

#endif
