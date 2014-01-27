custom_drivers
==============

Simple custom linux kernel drivers

####erin.c: static msg producer and sink device

1. Compile the driver into a module, which can be loaded
   as a character device.  This will produce a .ko file.
    $ cd drivers && make

2. Make a new device node
    $ sudo mknod /dev/erin c 250 0
    $ sudo chmod a+r+w /dev/erin

3. Insert a module into the linux kernel
    $ sudo insmod erin.ko

4. Check that the module is loaded
    $ cat /proc/modules | grep erin
 
5. Check that the module is listed as a character device
    $ cat /proc/devices | grep erin

... If something goes wrong, and your kernel doesn't have
CONFIG_MODULE_FORCE_UNLOAD set, you have no choice but to reboot.

