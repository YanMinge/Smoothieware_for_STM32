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
PROJECT                          := Blink
DEVICES                          ?= TEST_F407ZG
PROJECT_ROOT                     := ../../../
GCC4MBED_DIR                     := ../..
NO_FLOAT_SCANF                   := 1
NO_FLOAT_PRINTF                  := 1
MBED_SRC_ROOT                    := $(GCC4MBED_DIR)/mbed-os

# This program is so simple, it doesn't need any RTOS functionality.
MBED_OS_ENABLE := 0

# add mbed-os patch
include ./add-mbed-os-patch.mk

include $(GCC4MBED_DIR)/build/gcc4mbed.mk
