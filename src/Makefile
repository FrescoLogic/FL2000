# Makefile
#
# (c)Copyright 2017, Fresco Logic, Incorporated.
#
# Purpose:
#
# NOTE: DO NOT SEND THIS FILE OUTSIDE OF FRESCO LOGIC.
#

fl2000-y := fl2000_module.o
fl2000-y += fl2000_bulk.o
fl2000-y += fl2000_ioctl.o
fl2000-y += fl2000_render.o
fl2000-y += fl2000_dev.o
fl2000-y += fl2000_dongle.o
fl2000-y += fl2000_big_table.o
fl2000-y += fl2000_i2c.o
fl2000-y += fl2000_register.o
fl2000-y += fl2000_monitor.o
fl2000-y += fl2000_desc.o
fl2000-y += fl2000_interrupt.o
fl2000-y += fl2000_compression.o
fl2000-y += fl2000_surface.o
fl2000-y += fl2000_fops.o
fl2000-y += fl2000_hdmi.o

ifdef CONFIG_USB_FL2000

obj-$(CONFIG_USB_FL2000) := fl2000.o

else

obj-m := fl2000.o

#KERNEL_PATH = ../../../kernel/linux-3.10.0-327.13.1.el7
#KERNEL_PATH = /usr/src/kernels/3.10.0-327.13.1.el7.x86_64
#KERNEL_PATH = /usr/src/kernels/`uname -r`
KERNEL_PATH = /usr/src/linux-headers-`uname -r`

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
	rm -f Module.symvers

endif


