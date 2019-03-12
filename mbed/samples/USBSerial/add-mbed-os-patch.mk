# Command line tools that are different between *nix and Windows.
ifeq "$(OS)" "Windows_NT"
SHELL      =cmd.exe
REMOVE     =del /q
REMOVE_DIR =rd /s /y
COPY       =copy
COPYDIR    =echo d | xcopy
CAT        =type
MKDIR      =mkdir
MKFILE = type nul >
QUIET      =>nul 2>nul & exit 0
NOSTDOUT   =>nul
else
REMOVE     =rm
REMOVE_DIR =rm -r -f
COPY       =cp
COPYDIR    =cp
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

MBED_OS_PATCH_PATH := $(GCC4MBED_DIR)/mbed-os-patch
MBED_OS_PATH := $(GCC4MBED_DIR)/mbed-os
MBED_OS_PATH_ENABLE := 1

export MBED_OS_PATCHED_FLAG := $(MBED_OS_PATCH_PATH)/mbed-os-patched

#$(info MBED_OS_PATCH_PATH: $(MBED_OS_PATCH_PATH))
#$(info MBED_OS_PATCHED_FLAG: $(MBED_OS_PATCHED_FLAG))

.PHONY: all


all: copy

copy: copy_mbed_os_patch 

#Copy and replace mbed-os's code
#Check if mpy-cross is compiled, compile it if it has not yet been compiled
copy_mbed_os_patch:
ifeq ($(MBED_OS_PATCHED_FLAG), $(wildcard $(MBED_OS_PATCHED_FLAG)))
	@echo  mbed-os patch has been copied
else
	@echo  mbed-os patch has not been copied
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_pcd.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_pcd.c)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/sdio_device.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/sdio_device.h) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4)
	$(COPYDIR) $(call convert-slash, $(MBED_OS_PATCH_PATH)/after/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG/*.*) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG)
	$(MKFILE) $(call convert-slash, $(MBED_OS_PATCHED_FLAG))
endif

clean: clean_mbed_os_patch

#Clean and replace mbed-os's code
clean_mbed_os_patch:
ifeq ($(MBED_OS_PATCHED_FLAG), $(wildcard $(MBED_OS_PATCHED_FLAG)))
	@echo  mbed-os patch has been copied
	$(REMOVE) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/sdio_device.c)
	$(REMOVE) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/sdio_device.h)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/device/TOOLCHAIN_GCC_ARM/STM32F407XG.ld)
	$(COPY) $(call convert-slash, $(MBED_OS_PATCH_PATH)/before/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_pcd.c) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/device/stm32f4xx_hal_pcd.c)
	$(REMOVE_DIR) $(call convert-slash, $(MBED_OS_PATH)/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407xG/TARGET_TEST_F407ZG)
	$(REMOVE) $(call convert-slash, $(MBED_OS_PATCHED_FLAG))
else
	@echo  mbed-os patch has not been copied
endif