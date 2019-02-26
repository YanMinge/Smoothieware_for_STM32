1. The TEST_F407ZG is a new device that added in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG`

2. In the hardware I tested the sample code, the card detect pin is `PC_13`

3. The SERIAL_TX, SERIAL_RX is defined in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG\PinNames.h`

4. MBED_CONF_TARGET_STDIO_UART_TX and  MBED_CONF_TARGET_STDIO_UART_RX is defined in `Smoothieware_for_STM32\mbed\src\mbed_config.h`

5. If you want to observe the output log of the SDIO code, you need to use the BuildShellDebug terminal to compile the project. Also make sure that the pins of the serial port you are using are paired correctly.

6. To output the log, the `SD_DBG` In file `SDIOBlockDevice.cpp` also should set to `1`