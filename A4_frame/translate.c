#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h> /* printk(), min() */
#include <linux/sched.h>
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h>   /* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/module.h>

#include "trans.h" /* local definitions */

static int translate_bufsize = 40;
module_param(translate_bufsize, int, S_IRUGO);

static int translate_shift = 3;
module_param(translate_shift, int, S_IRUGO);

static struct trans trans0; 
static struct trans trans1; 

/*
 * The file operations for the trans device
 */
struct file_operations trans_fops = {
    .owner = THIS_MODULE,
    .llseek = no_llseek,
    .read = trans_read,
    .write = trans_write,
    .open = trans_open,
    .release = trans_release,
};

static int trans_init_module(void)
{
        int result;
        dev_t dev = 0;

        PDEBUG("trans_init_module started \n");

        sema_init(&trans0.sem, 1);
        sema_init(&trans1.sem, 1); // Anzahl der zu initialisierenden Semaphoren
        // init all elements auf trans_dev

        trans0.transPuffer = ḱmalloc(sizeof(char) * translate_bufsize, GFP_KERNEL);
        trans1.transPuffer = ḱmalloc(sizeof(char) * translate_bufsize, GFP_KERNEL);

        // TODO fixen: trans0/1 struct Elemente einrichten

        // Dynamic allocation of device numbers
        // Parameters dev: first dev
        // TRANS_MINOR : first minor number
        // 1 : number of devices that should be allocated
        // "trans" : number of device in /proc/devices
        result = alloc_chrdev_region(&dev, TRANS_MINOR, 1, "trans");
        trans_dev.major = MAJOR(dev);
        trans_dev.minor = MINOR(dev);
        printk(KERN_ALERT "after alloc_chrdev_region : major %d minor %d \n", trans_dev.major, trans_dev.minor);

        if (result < 0)
        {
                printk(KERN_WARNING "trans: can't get major %d\n", trans_dev.major);
                return result;
        }

        result = trans_setup_cdev(&(trans_dev.cdev), dev);
        if (result)
        {
                printk(KERN_WARNING "trans: can't generate cdev \n");
                return result;
        }

        printk(KERN_ALERT "trans installed major = %d minor = %d \n", trans_dev.major, trans_dev.minor);
        return 0;
}

/*
 * Set up a cdev entry.
 */
// TODO muss hier noch was gemacht werden?
static int trans_setup_cdev(struct cdev *cdev, dev_t devno)
{
        int error;

        // The kernel will generate a cdev structures. This structure
        // repesents a device within the kernel.
        cdev_init(cdev, &trans_fops);
        cdev->owner = THIS_MODULE;
        cdev->ops = &trans_fops;
        error = cdev_add(cdev, devno, 1);
        // Now the device is alive.
        printk(KERN_ALERT "cdev_add: major %d minor %d result %d \n", MAJOR(devno), MINOR(devno), error);

        /* Fail gracefully if need be */
        if (error)
                printk(KERN_NOTICE "Error %d adding translate %d", error, devno);
        return error;
}

static int trans_open(struct inode *inode, struct file *filp)
{
        unsigned int minor = iminor(filp->f_inode);
        switch (minor)
        {
        case 0:
                // TODO: was passiert bei nicht initialisiertem Semaphore
                if (down_interruptible(&trans0.sem) != 0)
                {
                        printk(KERN_ALERT "trans0 : the device has been opened by some other device\n");
                        return -EBUSY;
                }
                printk(KERN_INFO "trans0 : device opened succesfully\n");
                break;
        case 1:
                if (down_interruptible(&trans1.sem) != 0)
                {
                        printk(KERN_ALERT "trans1 : the device has been opened by some other device\n");
                        return -EBUSY;
                }
                printk(KERN_INFO "trans1 : device opened succesfully\n");
                break;
        default:
                printk(KERN_INFO "trans : invalid minor number (%d) while opening\n", minor);
                return -EFAULT;
        }
        // nonseekable_open informs the kernel that device does not support llseek
        return nonseekable_open(inode, filp);
}

static int trans_release(struct inode *inode, struct file *filp)
{
        unsigned int minor = iminor(filp->f_inode);
        switch (minor)
        {
        case 0:
                up(&trans0.sem);
                break;
        case 1:
                up(&trans1.sem);
                break;
        default:
                printk(KERN_INFO "trans : invalid minor number (%d) at release\n", minor);
                return -EFAULT
        }
        return 0;
}

/*
 * Data management: read and write
 */
//TODO caeser einbauen
static ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
        // AUSGABE
        unsigned int minor = iminor(filp->f_inode);

        int maxbytes;      /* maximum bytes that can be read from f_pos to translate_bufsize*/
        int bytes_to_read; /* gives the number of bytes to read*/
        int bytes_read;    /* number of bytes actually read*/
        maxbytes = translate_bufsize - *f_pos;
        if (maxbytes > count)
                bytes_to_read = count;
        else
                bytes_to_read = maxbytes;
        if (bytes_to_read == 0)
                printk(KERN_INFO "trans%d : Reached the end of the device\n", minor);

        switch (minor)
        {
        case 0:
                bytes_read = bytes_to_read - copy_to_user(buf, trans0.transPuffer + *f_pos, bytes_to_read);
                break;
        case 1:
                bytes_read = bytes_to_read - copy_to_user(buf, trans1.transPuffer + *f_pos, bytes_to_read);
                break;
        default:
                printk(KERN_INFO "trans : invalid minor number (%d) while writing\n", minor);
                return -EFAULT;
        }
        printk(KERN_INFO "trans : device has been read %d\n", bytes_read);
        *f_pos += bytes_read;
        return bytes_read;
}

//TODO caeser einbauen
static ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
        // EINGABE
        unsigned int minor = iminor(filp->f_inode);

        int maxbytes;       /* maximum bytes that can be read from ppos to BUFFER_SIZE*/
        int bytes_to_write; /* gives the number of bytes to write*/
        int bytes_writen;   /* number of bytes actually writen*/
        maxbytes = translate_bufsize - *f_pos;
        if (maxbytes > count)
                bytes_to_write = count;
        else
                bytes_to_write = maxbytes;

        switch (minor)
        {
        case 0:
                if(strlen(trans0.transpuffer) >= translate_bufsize-bytes_to_write)     {
                        printk(KERN_INFO "trans%d : full buffer during writing\n", minor);
                        return 0;
                }
                //TODO Bei anderen ergänzen; wenn noch 3 frei, aber 4 geschrieben werden sollen -> 3 oder keins schreiben?
                bytes_writen = bytes_to_write - copy_from_user(trans0.transPuffer + *f_pos, buf, bytes_to_write);
                break;
        case 1:
                bytes_writen = bytes_to_write - copy_from_user(trans1.transPuffer + *f_pos, buf, bytes_to_write);
                break;
        default:
                printk(KERN_INFO "trans : invalid minor number (%d) while reading\n", minor);
                return -EFAULT;
        }
        printk(KERN_INFO "trans%d : device has been written %d bytes\n", minor, bytes_writen);
        *f_pos += bytes_writen;
        return bytes_writen;
}

char caeser_off(char key, int offset)
{
        if (index(ALPHABET, key) == -1)
                return key;
        else
                return ALPHABET[(index(ALPHABET, key) - (offset * sizeof(char))) % 53];
}

char caeser_on(char key, int offset)
{
        if (index(ALPHABET, key) == -1)
                return key;
        else
                return ALPHABET[(index(ALPHABET, key) + (offset * sizeof(char))) % 53];
}

static void trans_cleanup_module(void)
{
        dev_t dev0 = MKDEV(trans0.major, trans0.minor);
        dev_t dev1 = MKDEV(trans1.major, trans1.minor);

        //TODO nur puffer oder gesamten struct freigeben?
        kfree(trans0.transPuffer);
        kfree(trans1.transPuffer);

        /* cleanup_module is never called if registering failed */
        cdev_del(&trans0.cdev);
        cdev_del(&trans1.cdev);
        unregister_chrdev_region(dev0, 1); // Anzahl der zu löschenden device nummern
        unregister_chrdev_region(dev1, 1);
        printk(KERN_ALERT "trans %d.%d and %d.%d deinstalled", trans0.major, trans0.minor, trans1.major, trans1.minor);
}

module_init(trans_init_module);    // Function that will be called for loading trans module
module_exit(trans_cleanup_module); // Function that will be called for removing trans module

// EOF