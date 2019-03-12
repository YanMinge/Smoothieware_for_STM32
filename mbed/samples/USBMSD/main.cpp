#include "mbed.h"
#include "SDIOBlockDevice.h"
#include "USBMSD_SDBD.h"
#include "USBSerial.h"

// Physical block device, can be any device that supports the BlockDevice API
SDIOBlockDevice bd(PC_13);

// USB MSD 
USBMSD_SDBD msd(&bd);

extern serial_t stdio_uart; 

// main() runs in its own thread in the OS
// (note the calls to Thread::wait below for delays)
int main()
{
	//uint8_t *read_block = new (std::nothrow) uint8_t[512];
    serial_init(&stdio_uart, SERIAL_TX, SERIAL_RX);  //重定向到 Serial1，也可以重定向到 Serial2
    printf("\r\n--- Mbed OS SD card as USB MSD example ---\r\n");

    printf("Starting MSD...\r\n");
    msd.disk_initialize();

    uint32_t read_size = bd.get_read_size();
    uint32_t all_size = bd.size();
    printf("read_size(%lu), all_size(%lu)\r\n", read_size, all_size);

    printf("msd connect\r\n");
    bool connectOK = msd.connect();
    DigitalOut led1(LED1);

    while (true)
    {
        if(!connectOK)
        {
           msd.disk_initialize();
           uint32_t read_size = bd.get_read_size();
           uint32_t all_size = bd.size();
           printf("read_size(%lu), all_size(%lu)\r\n", read_size, all_size);

           printf("msd connect\r\n");
           connectOK = msd.connect();
           printf("connectOK:%s\r\n", (connectOK ? "OK" : "Fail :("));
           wait_ms(1000);
        }
        wait_ms(500);
        led1 = !led1;
    }
}