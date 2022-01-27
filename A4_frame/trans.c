/*
 * trans river 
 *
 * based on the scull driver of 
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 * 
 * Simple frame for starting driver development
 *
 */
 
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* printk(), min() */
#include <linux/sched.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>


#include "trans.h"		/* local definitions */

MODULE_AUTHOR("Franz Korf");
MODULE_LICENSE("Dual BSD/GPL");

/*
 * private data of trans module
 */
struct trans{       
        struct semaphore sem;              // mutual exclusion semaphore for this structure 
        struct cdev cdev;                  // Char device structure 
        int major;
        int minor;
	// further data uses by the driver
};

/* module parameters 
 * Parameter, global variable and name used in module_param must be equal
 */
int test_param  = -1;                        // default time returned by read
module_param(test_param,  int, S_IRUGO);     // This macro must be called outside any function	
// further paremeters

static struct trans trans_dev; // struct that contains all data of trans device

/*
 * Open and close
 */

static int trans_open(struct inode *inode, struct file *filp) {
        // nonseekable_open informs the kernel that device does not support llseek
	return nonseekable_open(inode, filp);
}

static int trans_release(struct inode *inode, struct file *filp) {
	return 0;
}

/*
 * Data management: read and write
 */

static ssize_t trans_read (struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	return 0;
}

static ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	return 0;
}

/*
 * The file operations for the trans device
 */
struct file_operations trans_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.read =		trans_read,
	.write =	trans_write,
	.open =		trans_open,
	.release =	trans_release,
};

/*
 * Set up a cdev entry.
 */
static int trans_setup_cdev(struct cdev *cdev, dev_t devno) {
    int error;

    // The kernel will generate a cdev structures. This structure
    // repesents an device within the kernel.
    cdev_init(cdev, &trans_fops); 
    cdev->owner = THIS_MODULE;
    cdev->ops = &trans_fops;
    error = cdev_add (cdev, devno, 1);
    // Now the device is alive.
    printk(KERN_ALERT "cdev_add: major %d minor %d result %d \n", MAJOR(devno), MINOR(devno), error);
    
    /* Fail gracefully if need be */
    if (error) printk(KERN_NOTICE "Error %d adding translate %d", error, devno);
    return error;
}


/*
 * Initialize the trans dev 
 */

static int trans_init_module(void) {
	int result;
        dev_t dev = 0;

        PDEBUG("trans_init_module started \n");

        init_MUTEX(&trans_dev.sem);
	// init all elements auf trans_dev

        //  Dynamic allocation of device numbers 
        //  Parameters dev: first dev
        //                  TRANS_MINOR : first minor number
        //                  1 : number of devices that should be allocated
        //                  "trans" : number of device in /proc/devices
	result = alloc_chrdev_region(&dev, TRANS_MINOR, 1, "trans");
	trans_dev.major = MAJOR(dev);
	trans_dev.minor = MINOR(dev);
        printk(KERN_ALERT "after alloc_chrdev_region : major %d minor %d \n", trans_dev.major, trans_dev.minor);

	if (result < 0) {
     	    printk(KERN_WARNING "trans: can't get major %d\n", trans_dev.major);
	    return result;
	}

        result = trans_setup_cdev(&(trans_dev.cdev),dev);
        if (result) {
       	    printk(KERN_WARNING "trans: can't generate cdev \n");
            return result;
       }

       printk(KERN_ALERT "trans installed major = %d minor = %d \n", trans_dev.major, trans_dev.minor);
       return 0;
}

static void trans_cleanup_module(void) {
   dev_t devno = MKDEV(trans_dev.major, trans_dev.minor);

   /* cleanup_module is never called if registering failed */
   cdev_del(&trans_dev.cdev);
   unregister_chrdev_region(devno, 1); // unregister 1 dev number pair 
   printk(KERN_ALERT "trans deinstalled major = %d minor = %d \n", trans_dev.major, trans_dev.minor);
}

module_init(trans_init_module);     // Function that will be called for loading trans module
module_exit(trans_cleanup_module);  // Function that will be called for removing trans module 

// EOF

