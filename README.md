# Expected accomplishment #

1. Multi-chip platform support using the latest mbed-os source code.
2. Isolate the main code of Smoothieware from the hardware.
3. Implement the function support for STM32F407.
4. Simplify the code porting.

# Current Progress #

1. 2019-01-17: Completed support for the latest mbed-os build environment(2019-01-10 merge id: 2454b25eba55b05b6adaf1258faf61e0818f51ae).
2. 2019-01-18: The first version that can be compiled.
3. 2019-01-21: Add simple support for STM32F407.
4. 2019-01-24: Add USB serial device support.
5. 2019-02-21: Use submodule to manage mbed-os code

# Installation and compilation instructions #
1. Clone code with `git clone --recursive https://github.com/YanMinge/Smoothieware_for_STM32`
3. Use win_install.cmd or linux_install to install Compiler Environment
5. Enter the directory `Smoothieware_for_STM32\src` from the terminal BuildShell.cmd or BuildShell
6. Enter `make` to Compiled code
7. When compiling for the first time, If there is a mbed-os patch file, the patch operation will be performed first, so you needs to be executed `make` again.

# Development support #

If you have a similar idea and want to collaborate on development, you can contact me at myan@makeblock.com
