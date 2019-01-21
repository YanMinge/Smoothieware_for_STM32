PROJECT         := Blink
DEVICES         := TEST_F407ZG
PROJECT_ROOT    := ..
GCC4MBED_DIR    := $(PROJECT_ROOT)/mbed
NO_FLOAT_SCANF  := 1
NO_FLOAT_PRINTF := 1
MBED_SRC_ROOT   := $(GCC4MBED_DIR)/mbed-os
BOARD           := PRNTR_V1_407
MBED_CUSTOM     := 1

MBED_OS_ENABLE := 0
DEFAULT_SERIAL_BAUD_RATE ?= 115200

# Set to 1 to tag each heap allocation with the caller's return address.
# NOTE: Can't be enabled with latest build as not compatible with newlib nano.
HEAP_TAGS=0

# Set to 1 configure MPU to disable write buffering and eliminate imprecise bus faults.
WRITE_BUFFER_DISABLE=0

# Set to non zero value if you want checks to be enabled which reserve a
# specific amount of space for the stack.  The heap's growth will be
# constrained to reserve this much space for the stack and the stack won't be
# able to grow larger than this amount.
STACK_SIZE=0x1000

# Drivers that are not yet supported.
export NONETWORK = 1
export DISABLESD = 1
export DISABLEUSB = 1
export DISABLEMRI = 1
export NOUSBDEVICE = 1

DEFAULT_VERSION = 1.0.0.0

# Generate a git version string
__DEFAULT_VERSION_STRING__ := ~11myvar2
ifneq "$(OS)" "Windows_NT"
__GITVERSIONSTRING__ := $(shell ./generate-version.sh)
ifeq "$(__GITVERSIONSTRING__)" "$(__DEFAULT_VERSION_STRING__)"
__GITVERSIONSTRING__ := $(DEFAULT_VERSION)
endif
else
__GITVERSIONSTRING__ := $(shell generate-version.bat)
ifeq "$(__GITVERSIONSTRING__)" "$(__DEFAULT_VERSION_STRING__)"
__GITVERSIONSTRING__ := $(DEFAULT_VERSION)
endif
$(info Version:   $(__GITVERSIONSTRING__))
endif

DEFINES += -D$(DEVICES)
DEFINES += -D$(BOARD)
DEFINES += -D__STM32F4__
DEFINES += -D__GITVERSIONSTRING__=\"$(DEFAULT_VERSION)\"

# Use c++11 features for the checksums and set default baud rate for serial uart
DEFINES += -DCHECKSUM_USE_CPP -DDEFAULT_SERIAL_BAUD_RATE=$(DEFAULT_SERIAL_BAUD_RATE)

ifneq "$(STEPTICKER_DEBUG_PIN)" ""
# Set a Pin here that toggles on end of move
DEFINES += -DSTEPTICKER_DEBUG_PIN=\"$(STEPTICKER_DEBUG_PIN)\"
endif

# include an optional default set of excludes
# add any modules that you do not want included in the build
# e.g for a CNC machine
#export EXCLUDE_MODULES = tools/laser tools/temperaturecontrol tools/extruder
-include default_excludes.mk

# override any default excludes by setting NODEFAULTEXCLUDES=1
ifneq "$(NODEFAULTEXCLUDES)" "1"
# if nothing set in the default_excludes.mk file then handle some default excludes for different builds

ifeq "$(EXCLUDE_MODULES)" ""

ifeq "$(CNC)" "1"
# CNC build excludes these
export EXCLUDE_MODULES = tools/filamentdetector tools/scaracal tools/temperaturecontrol tools/temperatureswitch tools/extruder
else
# 3D build excludes these
export EXCLUDE_MODULES = tools/drillingcycles tools/spindle
endif

endif
endif

ifneq "$(INCLUDE_MODULE)" ""
export EXCLUDED_MODULES = $(filter-out $(INCLUDE_MODULE),$(EXCLUDE_MODULES))
else
export EXCLUDED_MODULES = $(EXCLUDE_MODULES)
endif

ifneq "$(EXCLUDED_MODULES)" ""
$(info **NOTE** Excluding modules $(EXCLUDED_MODULES))
endif

ifneq "$(AXIS)" ""
DEFINES += -DMAX_ROBOT_ACTUATORS=$(AXIS)
endif

# define the number of primary (cartesian) axis must be >= MAX_ROBOT_ACTUATORS
ifneq "$(PAXIS)" ""
DEFINES += -DN_PRIMARY_AXIS=$(PAXIS)
endif

# set to not compile in any network support
include ./main-ignore.mk

include $(GCC4MBED_DIR)/build/gcc4mbed.mk