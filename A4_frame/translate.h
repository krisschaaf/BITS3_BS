/*
 * trans.h -- definitions for the char module
 *
 * The code of this file is based on the code examples and the 
 * content of the book "Linux Device * Drivers" by Alessandro 
 * Rubini and Jonathan Corbet, published * by O'Reilly & Associates.   
 *
 * No warranty is attached; we cannot take responsibility 
 * for errors or fitness for use.
 *
 * Franz Korf HAW Hamburg
 */

#ifndef _TRANS_H_
#define _TRANS_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef TRANS_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_ALERT "trans: " fmt, ## args) 
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#ifndef TRANS_MINOR
#define TRANS_MINOR 0   /* dynamic major by default */
#endif

#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz"

struct trans
{
        struct semaphore sem; // mutual exclusion semaphore for this structure
        struct cdev cdev;     // Char device structure
        int major;
        int minor;
        char transPuffer[translate_bufsize];
};

// extern struct trans trans0; 
// extern struct trans trans1; 

// static int trans_open(struct inode *inode, struct file *filp);
// static int trans_release(struct inode *inode, struct file *filp);
// static ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
// static ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
// static int trans_setup_cdev(struct cdev *cdev, dev_t devno);
// static int trans_init_module(void);
// static void trans_cleanup_module(void);
// char caeser_off(char key, int offset);
// char caeser_on(char key, int offset);

#endif /* _TRANS_H_ */
