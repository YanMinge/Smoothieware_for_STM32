#include "mbed.h"
#include "USBSerial.h"
#include <stdlib.h>

//Virtual serial port over USB
USBSerial serial;
extern serial_t stdio_uart; 

DigitalOut l1(LED1);
DigitalOut l2(LED2);

uint8_t rx_table[64];
uint8_t g_read_bytes = 0;

void rx_callback(void)
{
    if(!serial.connected())
    {
        return;
    }
    else
    {
        uint8_t read_bytes = serial.available();
        g_read_bytes = read_bytes;
        printf("read_bytes:(%d)\r\n",read_bytes);
        memset(rx_table, 0x00, sizeof(rx_table)/sizeof(uint8_t));
        uint8_t i = 0;
        while(read_bytes)
        {
            uint8_t c = serial._getc();
            rx_table[i] = c;
            read_bytes--;
            i++;
        }
    }
}

int main(void)
{
    int i = 0;
    serial_init(&stdio_uart, SERIAL_TX, SERIAL_RX);  //Redirect printf to serial1
    serial.attach(rx_callback);
    while(1)
    {
        if(!serial.connected())
        {
            l2 = 1;
            printf("serial not connected\r\n");
        }
        else
        {
            printf("serial connected\r\n");
            l2 = 0;
        }
        l1 = !l1;
        printf("Hello\r\n");
        serial.printf("I am a virtual serial port: %d\r\n", i++);
        if(g_read_bytes != 0)
        {
            serial.writeBlock(rx_table, g_read_bytes);
        }
        g_read_bytes = 0;
        wait(0.1);
    }
}
