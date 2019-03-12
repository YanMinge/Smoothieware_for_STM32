#include "mbed.h"

Serial pc(SERIAL_TX, SERIAL_RX);

// Ticker flipper;
DigitalOut led1(LED1);
DigitalOut led2(LED2);


int main() {
    pc.baud(115200);
    pc.printf("Hello world!\r\n");
    while(1) {
        led1 = 1;
        led2 = 1;
        pc.printf("LED On\r\n");
        wait(0.5);
        led1 = 0;
        led2 = 0;
        pc.printf("LED Off \r\n");
        wait(0.5);
    }
}
