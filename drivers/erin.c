/*
 * erin.c          A sample character device
 *
 * Defining a custom character device can be as simple as:
 * - defining what will happen when your device file /dev/<name>
 *   is opened, released, read from, and written to.  Simply
 *   define methods for open, release, read, and write, and
 *   wrap them in a file_operations struct to be used when
 *   initializing the device.
 * - allocating the device in an init method, give it the
 *   file_operations.
 * - also in the init method, adding the device to the kernel
 *   with a major device number.
 * - deallocating the device and any other resources in a
 *   cleanup method.
 * - using linux/module.h to register the device as a module,
 *   which means it can be loaded into the kernel post-boot.
 *
 * Making a custom character driver can be as simple as:
 *
 * Using your custom character driver can be as simple as:
 */

#include <linux/module.h> // kernel support for initializing and exiting a module.
                          // From http://linux.die.net/lkmpg:
                          //     "Modules are pieces of code that can be loaded and 
                          //     unloaded into the kernel upon demand. They extend
                          //     the functionality of the kernel without the need to
                          //     reboot the system. For example, one type of module
                          //     is the device driver, which allows the kernel to
                          //     access hardware connected to the system."
#include <linux/kernel.h> // What's this for?
#include <linux/fs.h>     // For the file_operations struct, which we use to wrap
                          // the open, release, read, and write methods of our driver.



int erin_open(struct inode *inode, struct file *f)
{
  // 
}

int erin_release(struct inode *inode, struct file *f)
{
  //
}

ssize_t erin_read(struct file *f, char *buf, size_t count, loff_t *offp)
//
{
}

ssize_t erin_write(struct file *f, char *buf, size_t count, loff_t *offp)
{
}

/*
 * Defines the erin driver.
 */
struct file_operations fileops = {
 read: erin_read,
 write: erin_write,
 open: erin_open,
 release: erin_release
};

// A character device based on the erin driver.
struct cdev *character_device;

// dev_t is a type for holding the major and minor device numbers.
// We will dynamically query the kernel for a dev_t
// for the character device.
dev_t device_numbers;

// The major number of the character device.  This will
// be used to cleanup the device_number above.
static int major_number;

// 

/*
 * Initialize the character device.
 *
 * Steps include:
 * - 
 * -
 */
int erin_init(void)
{
  // obtain a standalone cdev structure at runtime.
  character_device = cdev_alloc();
  // this character device uses the erin driver file operations 
  character_device->ops = &fileops;
  // set the owner of the character device to this module.
  // THIS_MODULE is a macro defined in linux/module.h
  character_device->owner = THIS_MODULE;

  // dynamically ask the kernel for a device number for this
  // character device. Every linux device has a major and
  // minor number.
  int ret;
  ret = alloc_chrdev_region(&device_numbers, 0, 1, "erin");
  if (ret < 0) {
    printk("Oh shit. Major number allocation failed\n");
    return ret;
  }

  //
  major_number = MAJOR(device_numbers);
  printk("Major number for erin device is %d\n", major_number);
  //
  ret = cdev_add(character_device, MKDEV(major_number, 0), 1);
  if (ret < 0) {
    printk("Could not load erin character device\n");
    return ret;
  }
  return 0;
}

/*
 * Cleanup the character device.
 */
void erin_cleaup(void)
{
  //
  cdev_del(character_device);
  //
  unregister_chrdev_region(major_number, 1);
}

/*
 * Make the driver a kernel module,
 * which can be loaded post-boot.
 */
MODULE_LICENSE("GPL")
module_init(erin_init)
module_exit(erin_cleanup)
