# For building for the current running version of Linux
TARGET		:= $(shell uname -r)

KERNEL_MODULES	:= /lib/modules/$(TARGET)
KERNEL_BUILD	:= $(KERNEL_MODULES)/build
SYSTEM_MAP	:= $(shell if test -r /boot/System.map-$(TARGET) ; then echo /boot/System.map-$(TARGET) ; else echo $(KERNEL_BUILD)/System.map ; fi)

DRIVER := NEXCOM_CAN1

# Directory below /lib/modules/$(TARGET)/kernel into which to install
# the module:
MOD_SUBDIR = drivers/char

obj-m	:= $(DRIVER).o

MAKEFLAGS	+= --no-print-directory
EXTRA_CFLAGS	+= -DDEBUG

.PHONY: all install modules modules_install clean

all: modules

# Targets for running make directly in the external module directory:
modules clean:
	@$(MAKE) -C $(KERNEL_BUILD) M=$(CURDIR) $@

install: modules_install

modules_install:
	test -d $(KERNEL_MODULES)/kernel/$(MOD_SUBDIR) || mkdir $(KERNEL_MODULES)/kernel/$(MOD_SUBDIR)
	cp $(DRIVER).ko $(KERNEL_MODULES)/kernel/$(MOD_SUBDIR)
	depmod -a -F $(SYSTEM_MAP) $(TARGET)


