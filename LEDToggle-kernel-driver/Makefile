# Makefile for kernel modules and device tree script
# This file is a part of the BeagleLogic project

# Path to kernel sources relative to current directory
KSRC = /lib/modules/$(shell uname -r)/build

# Host name of BBB (like root@192.168.7.2)
BBHOST = root@192.168.7.2

# Module targets (run from host)
obj-m := ledtoggle.o

all:
	@make -C $(KSRC) M=$(PWD) modules

clean:
	@make -C $(KSRC) M=$(PWD) clean

# Deployment targets to be run only from BeagleBone environment as root!	

# Compile and install device tree
devicetree: devicetree-frombb

devicetree-frombb:
	dtc -O dtb -o BB-LEDTOGGLE-00A0.dtbo -b 0 -@ BB-LEDTOGGLE-00A0.dts

