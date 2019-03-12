1. Use add-mbed-os-patch.mk script to modify the code of mbed-os

2. The TEST_F407ZG is a new device that added in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG` 

3. The USB IO is defined in `Smoothieware_for_STM32\mbed\samples\USBSerial\USBDevice\targets\TARGET_STM\USBHAL_IP_OTGFSHS.h`

4. The SERIAL_TX, SERIAL_RX is defined in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG\PinNames.h` 

5. MBED_CONF_TARGET_STDIO_UART_TX and  MBED_CONF_TARGET_STDIO_UART_RX is defined in `Smoothieware_for_STM32\mbed\src\mbed_config.h` 

6. If you want to observe the output log of the SDIO code, you need to use the BuildShellDebug terminal to compile the project. Also make sure that the pins of the serial port are paired correctly

7. To output the log of SDIOBlockDevice, the `SD_DBG` In file `SDIOBlockDevice.cpp` also should set to `1`