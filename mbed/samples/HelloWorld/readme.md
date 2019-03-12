1. Use add-mbed-os-patch.mk script to modify the code of mbed-os

2. The TEST_F407ZG is a new device that added in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG`

3. The LED1, LED2 is defined in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG\PinNames.h` 

4. The SERIAL_TX, SERIAL_RX is defined in `Smoothieware_for_STM32\mbed\mbed-os\targets\TARGET_STM\TARGET_STM32F4\TARGET_STM32F407xG\TARGET_TEST_F407ZG\PinNames.h`

5. MBED_CONF_TARGET_STDIO_UART_TX and  MBED_CONF_TARGET_STDIO_UART_RX is defined in `Smoothieware_for_STM32\mbed\src\mbed_config.h`