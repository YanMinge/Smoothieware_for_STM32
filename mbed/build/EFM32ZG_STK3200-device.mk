# This target makefile was automatically generated by mbedUpdater.

# Device for which the code should be built.
MBED_DEVICE        := EFM32ZG_STK3200

# Can skip parsing of this makefile if user hasn't requested this device.
ifeq "$(findstring $(MBED_DEVICE),$(DEVICES))" "$(MBED_DEVICE)"

# Compiler flags which are specifc to this device.
TARGETS_FOR_DEVICE := $(BUILD_TYPE_TARGET) TARGET_UVISOR_UNSUPPORTED TARGET_32K TARGET_CORTEX_M TARGET_EFM32 TARGET_EFM32ZG TARGET_EFM32ZG222F32 TARGET_EFM32ZG_STK3200 TARGET_LIKE_CORTEX_M0 TARGET_M0P TARGET_SL_AES TARGET_Silicon_Labs
FEATURES_FOR_DEVICE :=
PERIPHERALS_FOR_DEVICE := DEVICE_ANALOGIN DEVICE_ERROR_PATTERN DEVICE_I2C DEVICE_I2CSLAVE DEVICE_I2C_ASYNCH DEVICE_INTERRUPTIN DEVICE_LOWPOWERTIMER DEVICE_PORTIN DEVICE_PORTINOUT DEVICE_PORTOUT DEVICE_PWMOUT DEVICE_RTC DEVICE_SERIAL DEVICE_SERIAL_ASYNCH DEVICE_SLEEP DEVICE_SPI DEVICE_SPISLAVE DEVICE_SPI_ASYNCH DEVICE_STDIO_MESSAGES
GCC_DEFINES := $(patsubst %,-D%,$(TARGETS_FOR_DEVICE))
GCC_DEFINES += $(patsubst %,-D%=1,$(FEATURES_FOR_DEVICE))
GCC_DEFINES += $(patsubst %,-D%=1,$(PERIPHERALS_FOR_DEVICE))
GCC_DEFINES += -D__CORTEX_M0PLUS -DARM_MATH_CM0PLUS -D__CMSIS_RTOS -D__MBED_CMSIS_RTOS_CM
GCC_DEFINES += -DEFM32ZG222F32 -DTRANSACTION_QUEUE_SIZE_SPI=0

# Value: HFXO for external crystal, HFRCO for internal RC oscillator
EFM32ZG_STK3200_CORE_CLOCK_SOURCE ?= "HFXO"
GCC_DEFINES += -DCORE_CLOCK_SOURCE=$(EFM32ZG_STK3200_CORE_CLOCK_SOURCE)

# Value: External crystal frequency in hertz
EFM32ZG_STK3200_HFXO_FREQUENCY ?= "24000000"
GCC_DEFINES += -DHFXO_FREQUENCY=$(EFM32ZG_STK3200_HFXO_FREQUENCY)

# Value: LFXO for external crystal, LFRCO for internal RC oscillator, ULFRCO for internal 1KHz RC oscillator
EFM32ZG_STK3200_LOW_ENERGY_CLOCK_SOURCE ?= "LFXO"
GCC_DEFINES += -DLOW_ENERGY_CLOCK_SOURCE=$(EFM32ZG_STK3200_LOW_ENERGY_CLOCK_SOURCE)

# Value: External crystal frequency in hertz
EFM32ZG_STK3200_LFXO_FREQUENCY ?= "32768"
GCC_DEFINES += -DLFXO_FREQUENCY=$(EFM32ZG_STK3200_LFXO_FREQUENCY)

# Value: Frequency in hertz, must correspond to setting of hfrco_band_select
EFM32ZG_STK3200_HFRCO_FREQUENCY ?= "21000000"
GCC_DEFINES += -DHFRCO_FREQUENCY=$(EFM32ZG_STK3200_HFRCO_FREQUENCY)

# Value: One of _CMU_HFRCOCTRL_BAND_21MHZ, _CMU_HFRCOCTRL_BAND_14MHZ, _CMU_HFRCOCTRL_BAND_11MHZ, _CMU_HFRCOCTRL_BAND_7MHZ, _CMU_HFRCOCTRL_BAND_1MHZ. Be sure to set hfrco_clock_freq accordingly!
EFM32ZG_STK3200_HFRCO_FREQUENCY_ENUM ?= "_CMU_HFRCOCTRL_BAND_21MHZ"
GCC_DEFINES += -DHFRCO_FREQUENCY_ENUM=$(EFM32ZG_STK3200_HFRCO_FREQUENCY_ENUM)

# Pin to pull high for enabling the USB serial port
EFM32ZG_STK3200_EFM_BC_EN ?= "PA9"
GCC_DEFINES += -DEFM_BC_EN=$(EFM32ZG_STK3200_EFM_BC_EN)

C_FLAGS   := -mcpu=cortex-m0plus -mthumb
ASM_FLAGS := -mcpu=cortex-m0plus -mthumb
LD_FLAGS  := -mcpu=cortex-m0plus -mthumb

# Extra platform specific object files to link into file binary.
DEVICE_OBJECTS := 

# Version of MRI library to use for this device.
DEVICE_MRI_LIB := 

# Determine all mbed source folders which are a match for this device so that it only needs to be done once.
DEVICE_MBED_DIRS := $(call filter_dirs,$(RAW_MBED_DIRS),$(TARGETS_FOR_DEVICE),$(FEATURES_FOR_DEVICE))

# Linker script to be used.  Indicates what should be placed where in memory.
EFM32ZG_STK3200_LSCRIPT  ?= $(call find_target_linkscript,$(DEVICE_MBED_DIRS))
LSCRIPT := $(EFM32ZG_STK3200_LSCRIPT)

include $(GCC4MBED_DIR)/build/device-common.mk

else
# Have an empty rule for this device since it isn't supported.
.PHONY: $(MBED_DEVICE)

ifeq "$(OS)" "Windows_NT"
$(MBED_DEVICE):
	@REM >nul
else
$(MBED_DEVICE):
	@#
endif
endif # ifeq "$(findstring $(MBED_DEVICE),$(DEVICES))"...
