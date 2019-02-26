#include "mbed.h"
#include "SDIOBlockDevice.h"

// This will take the system's default block device
SDIOBlockDevice sdiobd(PC_13);
extern serial_t stdio_uart; 

int main() {
    serial_init(&stdio_uart, SERIAL_TX, SERIAL_RX);  //重定向到 Serial1，也可以重定向到 Serial2
    printf("sdio block device init\r\n");
    sdiobd.init();
    while(1) {
        wait(0.5);
    }
}
