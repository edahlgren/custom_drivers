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
 */

#include <linux/module.h>    // kernel support for initializing and exiting a module.
                             // From http://linux.die.net/lkmpg:
                             //     "Modules are pieces of code that can be loaded and 
                             //     unloaded into the kernel upon demand. They extend
                             //     the functionality of the kernel without the need to
                             //     reboot the system. For example, one type of module
                             //     is the device driver, which allows the kernel to
                             //     access hardware connected to the system."
#include <linux/kernel.h>    // What's this for?
#include <linux/fs.h>        // For the file_operations struct, which we use to wrap
                             // the open, release, read, and write methods of our driver.
#include <linux/cdev.h>      // For the character device structure and allocation function.
#include <linux/uaccess.h>   // For the copy_to_user function, which copies a kernel buffer
                             // to a userspace buffer.
#include <linux/semaphore.h> // For creating, holding, and releasing a lock on this
                             // device, so that only one user can read/write from it at a time.

// Protect the character device file from
// multiple readers and writers.  This is
// not very sophisticated.
struct semaphore sem;

// The static msg this character device gives to consumers.
static char* MSG = "This is a static message from erin, coming from kernel memory.";
static int MSG_SIZE = sizeof(MSG);

int erin_open(struct inode *inode, struct file *f)
{
  // Try to acquire the semaphore, but stop trying
  // if the user sends a signal like ^C.
  if (down_interruptible(&sem)) {
    printk(KERN_INFO "/dev/erin: could not hold semaphore\n");
    return -1;
  }
  printk(KERN_INFO "/dev/erin: opened\n");
  return 0;
}

int erin_release(struct inode *inode, struct file *f)
{
  // Release the semaphore so other users of this
  // device can open it.
  up(&sem);
  printk(KERN_INFO "/dev/erin: closed\n");
  return 0;
}

ssize_t erin_read(struct file *f, char *buf, size_t count, loff_t *offp)
{
  // Called when a process tries to read from this device.
  // If the user is to get any data, we need to copy
  // any kernel buffers here to the user space buffer buf.
  return copy_to_user(buf, MSG, MSG_SIZE);
}

ssize_t erin_write(struct file *f, const char *buf, size_t count, loff_t *offp)
{
  // Consumes data like a sink, doing nothing with it.
  printk(KERN_INFO "/dev/erin: consumed %ld bytes\n", count);
  return count;
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

/*
 * Initialize the character device.
 */
int erin_init(void)
{
  int ret;

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
void erin_cleanup(void)
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
MODULE_LICENSE("GPL");
module_init(erin_init);
module_exit(erin_cleanup);
