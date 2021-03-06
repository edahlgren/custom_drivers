/*
 * block.c              A sample block device driver
 *
 * re spinlocks: http://www.makelinux.net/ldd3/chp-5-sect-5
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

// What happens if we tweek this?
#define KERNEL_SECTOR_SIZE 512

//
static const char DEVICE_NAME[] = "simple_block";
//
static const char DISK_NAME[] = "sbd0";  
//
static int LOGICAL_BLOCK_SIZE = 512;
//
static int NSECTORS = 1024;


// Where is request_queue defined?
static struct request_queue *queue;
//
static unsigned long size;
//
static u8 *data;
//
static struct gendisk *gdisk;

/*
 *
 */
void block_request(struct request_queue *q)
{
  struct request *req;
  unsigned long offset, nbytes;

  req = blk_fetch_request(q);
  while (req != NULL) {
    // Stop looping once we've exhausted the queue.
    // The kernel will call this function whenever
    // there is at least one element in the queue.

    // Check if we support handling this request.
    if (req == NULL || req->cmd_type != REQ_TYPE_FS) {
      // Declare our intention to handle no buffers
      // from this request.  We'll use an IO error
      // to signal that we don't accept requests that
      // aren't related to reading/writing to the
      // filesystem.
      blk_end_request_all(req, -EIO);
      continue;
    }
    
    // Handle the request.

    //
    offset = blk_rq_pos(req) * LOGICAL_BLOCK_SIZE;
    //
    nbytes = blk_rq_cur_sectors(req) * LOGICAL_BLOCK_SIZE;

    if (rq_data_dir(req)) {
      // Check that the write won't exceed the size of the block device.
      if ((offset + nbytes) <= size) {
	// Do write.
	memcpy(data + offset, req->buffer, nbytes);
      }
    } else {
      // Do read.
      memcpy(req->buffer, data + offset, nbytes);
    }

    // Declare our intention to end the request.
    // if buffers still need to be handled, blk_end_request_cur
    // will return true, and we'll continue handling this req.
    if (!blk_end_request_cur(req, 0)) {
      // If not, pop a new request off the queue
      req = blk_fetch_request(q);
    }
  }
}

// Why are we using a spin lock?
spinlock_t lock;

// major number of the device
static int major_number;

/*
 *
 */
int get_geo(struct block_device *dev, struct hd_geometry *geo)
{
  geo->cylinders = 8192;
  geo->heads = 4;
  geo->sectors = 16;
  geo->start = 0;
  return 0;
}

//
static struct block_device_operations block_ops = {
 owner: THIS_MODULE,
 getgeo: get_geo
};


/*
 *
 */
void block_cleanup(void)
{
  if (gdisk != NULL) {
    //
    del_gendisk(gdisk);
    //
    put_disk(gdisk);
  }

  if (major_number > 0) {
    //
    unregister_blkdev(major_number, DEVICE_NAME);
  }

  if (queue != NULL) {
    //
    blk_cleanup_queue(queue);
  }

  if (data != NULL) {
    //
    vfree(data);
  }
}

/*
 *
 */
int block_init(void)
{
  printk(KERN_INFO "initializing block device module\n");

  //
  spin_lock_init(&lock);

  //
  size = NSECTORS * LOGICAL_BLOCK_SIZE;
  data = vmalloc(size);
  if (data == NULL) {
    printk(KERN_INFO "block_init: could not malloc a block of size %lu\n", size);
    return -ENOMEM;
  }

  //
  queue = blk_init_queue(block_request, &lock);
  if (queue == NULL) {
    printk(KERN_INFO "block_init: could not initialize blk queue\n");
    block_cleanup();
    return -ENOMEM;
  }

  //
  blk_queue_logical_block_size(queue, LOGICAL_BLOCK_SIZE);

  //
  major_number = register_blkdev(major_number, DEVICE_NAME);
  if (major_number < 0) {
    printk(KERN_INFO "block_init: could not register blk device, major number=%d\n", major_number);
    block_cleanup();
    return -ENOMEM;
  }


  gdisk = alloc_disk(16);
  if (!gdisk) {
    printk(KERN_INFO "block_init: could not alloc gdisk\n");
    block_cleanup();
    return -ENOMEM;
  }

  //
  gdisk->major = major_number;
  //
  gdisk->first_minor = 0;
  //
  gdisk->fops = &block_ops;
  //
  gdisk->private_data = &data;
  //
  gdisk->queue = queue;
  //
  strcpy(gdisk->disk_name, DISK_NAME);

  //
  set_capacity(gdisk, 0);
  //
  add_disk(gdisk);
  printk(KERN_INFO "block_init: added gendisk\n");

  set_capacity(gdisk, NSECTORS);
  printk(KERN_INFO "block_init: set capacity on gendisk to %d sectors\n", NSECTORS);

  return 0;
}


MODULE_LICENSE("GPL");
module_init(block_init);
module_exit(block_cleanup);
