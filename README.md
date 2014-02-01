custom_drivers
==============

Simple custom linux kernel drivers

####kthreads.c: single suicidal kernel thread

1. Compile the task initialization and cleanup into
   a module.  This will produce a .ko file.

   <code>
   $ cd drivers/kthreads && make
   </code>

2. Insert the module into the kernel

   <code>
   $ sudo insmod kthreads.ko
   </code>

3. Check that the task is running and watch
   the task lifecycle

   <code>
   $ ps -ef | grep erin_thread
   $ tail -f /var/log/{messages,kernel,dmesg,syslog}
   </code>

####erin.c: static msg producer and sink device

1. Compile the driver into a module, which can be loaded
   as a character device.  This will produce a .ko file.

    <code>
    $ cd drivers/character_device && make
    </code>

2. Make a new device node

    <code>
    $ sudo mknod /dev/erin c 250 0<br>
    $ sudo chmod a+r+w /dev/erin
    </code>

3. Insert a module into the linux kernel

    <code>
    $ sudo insmod erin.ko
    </code>

4. Check that the module is loaded

    <code>
    $ cat /proc/modules | grep erin
    </code>
 
5. Check that the module is listed as a character device

    <code>
    $ cat /proc/devices | grep erin
    </code>

6. Watch what the device is doing

    <code>
    $ tail -f /var/log/{messages,kernel,dmesg,syslog}
    </code>

7. Try using it

    <code>
    $ cat /dev/erin<br>
    $ echo "hi" > /dev/erin
    </code>

... If something goes wrong, and your kernel doesn't have
CONFIG_MODULE_FORCE_UNLOAD set, you have no choice but to reboot.

