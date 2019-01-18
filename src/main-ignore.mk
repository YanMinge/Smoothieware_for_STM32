MAIN_IGNORE := ./TEST_F407ZG/%
MAIN_IGNORE += ./testframework/%
MAIN_IGNORE += ./libs/ChaNFS/%

ifeq "$(NONETWORK)" "1"
MAIN_IGNORE += ./libs/Network/%
DEFINES += -DNONETWORK
endif

ifeq "$(DISABLEMSD)" "1"
DEFINES += -DDISABLEMSD
endif
ifeq "$(DISABLESD)" "1"
DEFINES += -DDISABLESD
endif
ifeq "$(DISABLEUSB)" "1"
DEFINES += -DDISABLEUSB
endif

ifneq "$(DEVICE)" "LPC1768"
MAIN_IGNORE += ./libs/LPC17xx/%
endif

ifeq "$(NOUSBDEVICE)" "1"
MAIN_IGNORE += ./libs/USBDevice/%
DEFINES += -DNOUSBDEVICE
endif