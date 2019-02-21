# Command line tools that are different between *nix and Windows.
ifeq "$(OS)" "Windows_NT"
SHELL      =cmd.exe
REMOVE     =del /q
REMOVE_DIR =rd /s /q
COPY       =copy
CAT        =type
MKDIR      =mkdir
MKFILE = type nul >
QUIET      =>nul 2>nul & exit 0
NOSTDOUT   =>nul
else
REMOVE     =rm
REMOVE_DIR =rm -r -f
COPY       =cp
CAT        =cat
MKDIR      =mkdir -p
MKFILE = touch
QUIET      => /dev/null 2>&1 ; exit 0
NOSTDOUT   => /dev/null
endif

# Create macro which will convert / to \ on Windows.
ifeq "$(OS)" "Windows_NT"
define convert-slash
$(subst /,\,$1)
endef
else
define convert-slash
$1
endef
endif

MBED_OS_PATCH_PATH := ../mbed/mbed-os-patch
MBED_OS_PATH := ../mbed/mbed-os

MBED_OS_PATCHED_FLAG := $(MBED_OS_PATCH_PATH)/mbed-os-patched

#$(info MBED_OS_PATCH_PATH: $(MBED_OS_PATCH_PATH))
#$(info MBED_OS_PATCHED_FLAG: $(MBED_OS_PATCHED_FLAG))

all: copy_mbed_os_patch 

#Copy and replace mbed-os's code
#Check if mpy-cross is compiled, compile it if it has not yet been compiled
copy_mbed_os_patch:
ifeq ($(MBED_OS_PATCHED_FLAG), $(wildcard $(MBED_OS_PATCHED_FLAG)))
	@echo  mbed-os patch has been copied
else
	@echo  mbed-os patch has not been copied
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralNames.h) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralNames.h)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralPins.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralPins.c)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PinNames.h) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PinNames.h)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/system_clock.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/system_clock.c)
	$(MKFILE) $(MBED_OS_PATCHED_FLAG)
endif

clean: clean_mbed_os_patch

#Clean and replace mbed-os's code
clean_mbed_os_patch:
ifeq ($(MBED_OS_PATCHED_FLAG), $(wildcard $(MBED_OS_PATCHED_FLAG)))
	@echo  mbed-os patch has been copied
	$(REMOVE) $(MBED_OS_PATCHED_FLAG)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralNames.h) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralNames.h)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralPins.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PeripheralPins.c)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PinNames.h) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/PinNames.h)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/system_clock.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/system_clock.c)
else
	@echo  mbed-os patch has not been copied
endif