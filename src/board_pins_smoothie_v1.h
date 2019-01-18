#ifndef __BOARD_PINS_SMOOTHIE_V1_H__
#define __BOARD_PINS_SMOOTHIE_V1_H__

// The firmware has hard coded pins for two SPI channels.
// The configuration selects between the channels.
#define SPI1_MOSI P0_18
#define SPI1_MISO P0_17
#define SPI1_SCK  P0_15

#define SPI2_MOSI P0_9
#define SPI2_MISO P0_8
#define SPI2_SCK  P0_7

// The chip-select pin for the SD-Card is also hardcoded in the
// firmware.
#define SDCARD_CS P0_6

// Diagnostic LEDs used by Smoothieboard V1.
#define LED1    P1_18
#define LED2    P1_19
#define LED3    P1_20
#define LED4    P1_21
#define LED5    P4_28

// I2C bus used by the current control chip.
#define I2C_SDA p9
#define I2C_SCK p10

#define SERIAL0_TX USBTX
#define SERIAL0_RX USBRX

#define SERIAL1_TX p13
#define SERIAL1_RX p14

#define SERIAL2_TX p28
#define SERIAL2_RX p27

#define SERIAL3_TX p9
#define SERIAL3_RX p10

#endif // __BOARD_PINS_SMOOTHIE_V1_H__