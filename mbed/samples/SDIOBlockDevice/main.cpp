#include "mbed.h"
#include "SDIOBlockDevice.h"

// This will take the system's default block device
SDIOBlockDevice sdiobd(PC_13);
extern serial_t stdio_uart; 

int main() {
    serial_init(&stdio_uart, SERIAL_TX, SERIAL_RX);  //Redirect printf to serial1
    printf("sdio block device init\r\n");
    sdiobd.init();
    while(1) {
        wait(0.5);
    }
}
