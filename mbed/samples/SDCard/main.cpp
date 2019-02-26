#include "mbed.h"
#include "FATFileSystem.h"
#include "SDIOBlockDevice.h"
#include "File.h"
#include <stdlib.h>

// Test block device
#define BLOCK_SIZE 512
#define BLOCK_COUNT 128
#define TEST_SIZE BLOCK_SIZE

SDIOBlockDevice sdiobd(PC_13);
//SDIOBlockDevice sdiobd(PC_13);

extern serial_t stdio_uart; 

int main() {
	//sdiobd = new (std::nothrow) SDIOBlockDevice(PC_13);
	FATFileSystem::format(&sdiobd);
    serial_init(&stdio_uart, SERIAL_TX, SERIAL_RX);  //重定向到 Serial1，也可以重定向到 Serial2
    wait(0.5);
    printf("sdio block device init\r\n");
    FATFileSystem fs("fat");
    int err = fs.mount(&sdiobd);
    err = fs.mkdir("test_read_dir", S_IRWXU | S_IRWXG | S_IRWXO);
    err = fs.mkdir("test_read_dir/test_dir", S_IRWXU | S_IRWXG | S_IRWXO);

    uint8_t *buffer = new (std::nothrow) uint8_t[TEST_SIZE];
    // Fill with random sequence
    srand(1);
    for (int i = 0; i < TEST_SIZE; i++) {
        buffer[i] = 0xff & rand();
    }

    File file;
    err = file.open(&fs, "test_read_write2.dat", O_WRONLY | O_CREAT);
    ssize_t size = file.write(buffer, TEST_SIZE);
    printf("write sd card!\r\n");
    err = file.close();

    err = file.open(&fs, "test_read_write2.dat", O_RDONLY);
    for (int i = 0; i < TEST_SIZE; i++) {
        buffer[i] = 0xff;
    }
    size = file.read(buffer, TEST_SIZE);
    err = file.close();
    printf("read sd card, size = %d\r\n",size);
    for (int i = 0; i < size; i++) {
        printf("0x%x, ", buffer[i]);
    }
    while(1) {
        wait(0.5);
    }
}
