# Copyright 2017 Adam Green (http://mbed.org/users/AdamGreen/)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
PROJECT         := Blink
DEVICES         := TEST_F407ZG
PROJECT_ROOT    := ..
GCC4MBED_DIR    := $(PROJECT_ROOT)/mbed
NO_FLOAT_SCANF  := 1
NO_FLOAT_PRINTF := 1
MBED_SRC_ROOT   := $(GCC4MBED_DIR)/mbed-os

MBED_OS_ENABLE := 0

# Set to 1 to tag each heap allocation with the caller's return address.
# NOTE: Can't be enabled with latest build as not compatible with newlib nano.
HEAP_TAGS=0

# Set to 1 configure MPU to disable write buffering and eliminate imprecise bus faults.
WRITE_BUFFER_DISABLE=0

# Set to non zero value if you want checks to be enabled which reserve a
# specific amount of space for the stack.  The heap's growth will be
# constrained to reserve this much space for the stack and the stack won't be
# able to grow larger than this amount.
STACK_SIZE=3072

# Set to 1 to allow MRI debug monitor to take full control of UART0 and use it
# as a dedicated debug channel.  If you are using the USB based serial port for
# the console then this should cause you no problems.  Set MRI_BREAK_ON_INIT to
# 0 if you don't want to break into GDB at startup.
ENABLE_DEBUG_MONITOR?=0

# this is the default UART baud rate used if it is not set in config
# it is also the baud rate used to report any errors found while parsing the config file
DEFAULT_SERIAL_BAUD_RATE?=9600

ifeq "$(ENABLE_DEBUG_MONITOR)" "1"
# Can add MRI_UART_BAUD=115200 to next line if GDB fails to connect to MRI.
# Tends to happen on some Linux distros but not Windows and OS X.
MRI_UART=MRI_UART_0
MRI_BREAK_ON_INIT=1
MRI_SEMIHOST_STDIO=1
else
MRI_UART=MRI_UART_0 MRI_UART_SHARE
MRI_BREAK_ON_INIT=0
MRI_SEMIHOST_STDIO=0
endif

DEFAULT_VERSION = 1.0.0.0

# generate a git version string
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

DEFINES += -D__GITVERSIONSTRING__=\"$(DEFAULT_VERSION)\"

# use c++11 features for the checksums and set default baud rate for serial uart
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
# export EXCLUDE_MODULES = tools/filamentdetector tools/scaracal tools/temperaturecontrol tools/temperatureswitch tools/extruder
else
# 3D build excludes these
# export EXCLUDE_MODULES = tools/drillingcycles tools/spindle
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
#export NONETWORK = 1

include $(GCC4MBED_DIR)/build/gcc4mbed.mk