#!/usr/bin/make

DIRS = ./mbed/samples/Blink
DIRSCLEAN = $(addsuffix .clean,$(DIRS))

export DEVICES   := DISCO_F407VG

all:
	@ $(MAKE) -C ./mbed/samples/Blink
	@echo Building Smoothie
#@ $(MAKE) -C src

clean: $(DIRSCLEAN)

$(DIRSCLEAN): %.clean:
	@echo Cleaning $*
	@ $(MAKE) -C $*  clean

debug-store:
	@ $(MAKE) -C src debug-store

flash:
	@ $(MAKE) -C src flash

dfu:
	@ $(MAKE) -C src dfu

upload:
	@ $(MAKE) -C src upload

debug:
	@ $(MAKE) -C src debug

console:
	@ $(MAKE) -C src console

.PHONY: all $(DIRS) $(DIRSCLEAN) debug-store flash upload debug console dfu
