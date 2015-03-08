LEDToggle makes use of remoteproc driver to TOGGLE 8 GPIO pins 
from PRU0.

This project only aims at gaining better understanding of PRUSS and remoteproc.

It makes extensive use of BeagleLogic (https://github.com/abhishek-kakkar/BeagleLogic)
bindings to the pru-rpoc driver

INSTALLATION:

KERNEL MODULE
1. cd to LEDtoggle-kernel-driver
2. make
3. cp ledtoggle.ko /lib/modules/3.8.13-bone70/kernel/drivers/remoteproc
4. depmod -a

DEVICE TREE
5. make devicetree   ( run in same driver directory )
6. cp BB-LEDTOGGLE-00A0.dtbo /lib/firmware

FIRMWARE
7. cd to LEDtoggle-firmware
8. make
9. cp ledtoggle-pru0 /lib/firmware

TO RUN:
1. echo BB-LEDTOGGLE > /sys/devices/bone_capemgr.*/slots
2. modprobe ledtoggle
3. echo "1" > /sys/devices/virtual/misc/beaglelogic/ledtoggle
(Toggle following pins each time "1" is echoed to sysfs file)

pru0_r30_0 to pru0_r30_8 are toggled
pru0_r30_0 to pru0_r30_8 pinmux.pdf contains the pru0_r30_x (pru format) GPIOy_z (GPIO format) mappings.
beaglebone_headers.png has the pin number in GPIO format to header pin mapping.
