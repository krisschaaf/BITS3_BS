#include <linux / module.h>
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
#include <linux/vmalloc.h>

#include "trans.h" /* local definitions */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Franz Korf");

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

struct trans
{
    struct semaphore sem; // mutual exclusion semaphore for this structure
    struct semaphore grantAccess;   // sperrt, wenn Puffer leer ist 
    struct cdev cdev; // Char device structure
    int major;
    int minor;
    char *transPuffer;
    int actIndex;   // Index wo gerade zuletzt gearbeitet wurde im Puffer 
};

static int trans_init_module(void)
{
    int result;
    dev_t dev = 0;

    PDEBUG("trans_init_module started \n");

    // init all elements auf trans_dev
    sema_init(&trans0.sem, 1);
    sema_init(&trans1.sem, 1); // Anzahl der zu initialisierenden Semaphoren
    sema_init(&trans0.grantAccess, 1);
    sema_init(&trans1.grantAccess, 1);
    down(&trans0.grantAccess);  // Speicher ist leer, also keine Leserechte 
    down(&trans1.grantAccess);
    // Pufferspeicher wird allokiert
    trans0.transPuffer = kmalloc(translate_bufsize, GFP_KERNEL);
    trans1.transPuffer = kmalloc(translate_bufsize, GFP_KERNEL);

    // Dynamic allocation of device numbers
    // Parameters dev: first dev
    // TRANS_MINOR : first minor number
    // 1 : number of devices that should be allocated
    // "trans" : number of device in /proc/device

    result = alloc_chrdev_region(&dev, TRANS_MINOR, 2, "trans");
    trans0.major = MAJOR(dev); // dynamische Major Nummer
    trans1.major = MAJOR(dev);
    if (result < 0)
    {
        printk(KERN_WARNING "trans: can't get major %d\n", trans0.major);
        return result;
    }

    printk(KERN_ALERT "after alloc_chrdev_region : major %d minor %d and major %d minor %d\n", trans0.major, trans0.minor, trans1.major, trans1.minor);

    result = trans_setup_cdev(&(trans0.cdev), dev);
    if (result)
    {
        printk(KERN_WARNING "trans0: can't generate cdev \n");
        return result;
    }
    printk(KERN_ALERT "trans0 installed major = %d minor = %d \n", trans0.major, trans0.minor);

    return 0;
}

/*
 * Set up a cdev entry.
 */
static int trans_setup_cdev(struct cdev *cdev, dev_t devno)
{
    int error;

    // The kernel will generate a cdev structures. This structure
    // repesents a device within the kernel.
    cdev_init(cdev, &trans_fops);
    cdev->owner = THIS_MODULE;
    cdev->ops = &trans_fops;
    error = cdev_add(cdev, devno, 2);
    // Now the device is alive.
    printk(KERN_ALERT "cdev_add: major %d minor %d result %d \n", MAJOR(devno), MINOR(devno), error);

    /* Fail gracefully if need be */
    if (error)
        printk(KERN_NOTICE "Error %d adding translate %d", error, devno);
    return error;
}

//Beim Öffnen des jeweiligen Devices wird geprüft ob dieses schon exisitiert 
static int trans_open(struct inode *inode, struct file *filp)
{
    unsigned int minor = iminor(inode);
    switch (minor)
    {
    case 0:
        if (down_trylock(&trans0.sem) != 0)
        {
            printk(KERN_ALERT "trans0 : the device has been opened by some other device\n");
            return -EBUSY;
        }
        printk(KERN_INFO "trans0 : device opened succesfully\n");
        break;
    case 1:
        if (down_trylock(&trans1.sem) != 0)
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

//Wenn Device geschlossen wird, wird dessen Semaphor wieder freigegeben
static int trans_release(struct inode *inode, struct file *filp)
{
    unsigned int minor = iminor(inode;
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
        return -EFAULT;
    }
    return 0;
}

/*
 * Data management: read and write
 */
static ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // AUSGABE
    //Device bestimmmen, welches lesen will 
    unsigned int minor = iminor(filp->f_inode);
    if (minor != 1 || minor != 0)
    {
        printk(KERN_INFO "trans : invalid minor number (%d) while writing\n", minor);
        return -EFAULT;
    }
    int i = 0;
    int maxbytes;      
    int bytes_to_read; 
    int bytes_read;  
    maxbytes = translate_bufsize - *f_pos;
    //Maximal lesbare Bytes werden bestimmt 
    if (maxbytes > count)
        bytes_to_read = count;  //Wenn mehr Bytes gelesen werden sollen als Pufferspeicher ist wird nur so viel gelesen wie Speicher ist 
    else
        bytes_to_read = maxbytes;

    switch (minor)
    {
    case 0:
        // lesender Zugriff in blockiert bzw. wartet bei leerem Puffer
        if (bytes_to_read <= 0))
        {
            printk(KERN_INFO "trans0 : nothing to read in buffer, waiting\n");
            up(&trans0->sem);
            call(down_interruptible(&trans0->grantAccess));
            up(&trans0->grantAccess);
            call(down_interruptible(&trans0->sem));
        }
        //Bytes werden in den Puffer geschrieben 
        bytes_read = bytes_to_read - copy_to_user(buf, trans0->transPuffer + *f_pos, bytes_to_read);
        break;
    case 1:
        // lesender Zugriff in blockiert bzw. wartet bei leerem Puffer
        if (bytes_to_read <= 0))
        {
            printk(KERN_INFO "trans1 : nothing to read in buffer, waiting\n");
            up(&trans1->sem);
            call(down_interruptible(&trans1->grantAccess));
            up(&trans1->grantAccess);
            call(down_interruptible(&trans1->sem));
        }
        // lesender Zugriff in trans1 gibt Inhalt des Puffers zyklisch aus -> wenn am Ende angekommen, dann gebe Inhalt vom Anfang aus
        // Durch die Prüfung der MaxBytes eigentlich nicht erforderlich, aber in Aufgabenstellung nicht eindeutig
        for (; i < bytes_to_read; i++)
        {
            int index = (trans1.actIndex + 1) % bytes_to_read;
            trans1.transPuffer[index] = caeser_off(trans1.transPuffer[index, translate_shift);
            trans1.actIndex = (trans1.actIndex + 1) % translate_bufsize;
        }
        bytes_read = bytes_to_read - copy_to_user(buf, trans1->transPuffer + *f_pos, bytes_to_read);
        break;
    }
    printk(KERN_INFO "trans%d : device has been read %d\n", minor, bytes_read);
    *f_pos += bytes_read;
    return bytes_read;
}

static ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // EINGABE
    // Device bestimmen, welches schreiben will 
    unsigned int minor = iminor(filp->f_inode);
    if (minor != 1 || minor != 0)
    {
        printk(KERN_INFO "trans : invalid minor number (%d) while writing\n", minor);
        return -EFAULT;
    }
    // Um Semaphoren für paralleles Lesen und Schreiben zu setzen
    int emptyBuf;
    if(strlen(trans1.transPuffer) == 0) 
        emptyBuf = 1;
    else 
        emptyBuf = 0;
    
    int maxbytes;       
    int bytes_to_write; 
    int bytes_writen;   
    maxbytes = translate_bufsize - *f_pos;
    // Nur so viel schreiben, wie platz im puffer ist 
    if (maxbytes > count)
        bytes_to_write = count;
    else
        bytes_to_write = maxbytes;

    switch (minor)
    {
    case 0:
        bytes_writen = bytes_to_write - copy_from_user(trans0.transPuffer + *f_pos, buf, bytes_to_write);
        int i;
        i = 0;
        // while Schleife, da for-Schleife vom Kompiler aus irgendeinem Grund nicht akzeptiert wird 
        while (i < bytes_writen)
        {
            // schreibender Zugriff in trans0 schreibt Inhalt zyklisch in Puffer -> wenn am Ende angekommen, dann schreibe Inhalt am Anfang weiter
            // Durch die Prüfung der MaxBytes eigentlich nicht erforderlich, aber in Aufgabenstellung nicht eindeutig
            int index = (trans0.actIndex + 1) % bytes_writen;
            trans0.transPuffer[index] = caeser_on(trans0.transPuffer[index], translate_shift);
            trans0.actIndex = (trans0.actIndex + 1) % translate_bufsize;
            i++;
        }
        if( bytes_writen > 0 && emptyBuf)
            up(&trans0.grantAccess);      // Elemente zum Lesen sind vorhanden, also Sperre wieder freigeben
        else if (strl(trans0.transPuffer) == 0))
            call(down_trylock(&trans0.grantAccess));     // Sperre zum lesen setzen, da Buffer jetzt leer ist 
        break;
    case 1:
        // Bei bereits vollem Puffer wird die Funktion verlassen -> sollte nicht auftreten, da maxBytes das überprüft
        if (strlen(trans1.transPuffer) == translate_bufsize)
        {
            printk(KERN_INFO "trans1 : buffer full, escaping function\n");
            return 0;
        }
        bytes_writen = bytes_to_write - copy_from_user(trans1.transPuffer + *f_pos, buf, bytes_to_write);
        if( bytes_writen > 0 && emptyBuf)
            up(&trans1.grantAccess);    // Elemente zum Lesen sind vorhanden, also Sperre wieder freigeben
        else if (strl(trans1.transPuffer) == 0))
            call(down_trylock(&trans1.grantAccess));    // Sperre zum lesen setzen, da Buffer jetzt leer ist 
        break;
    }
    printk(KERN_INFO "trans%d : device has been written %d bytes\n", minor, bytes_writen);
    *f_pos += bytes_writen;


    return bytes_writen;
}

// caeser entschlüsseln, offset hier als Parameter -> könnte auch global genutzt werden 
char caeser_off(char key, int offset)
{
    int found = index(key);
    if (found == -1)
        return key;
    else
        return ALPHABET[found - (offset * (sizeof(char)) % 53)];
}

// caeser verschlüsseln, offset hier als Parameter -> könnte auch global genutzt werden 
char caeser_on(char key, int offset)
{
    int found = index(key);
    if (found == -1)
        return key;
    else
        return ALPHABET[found + (offset * (sizeof(char)) % 53)];
}

// gibt Index des übergebenen Keys in selbst definiertem Alphabet zurück 
int index(char key)
{
    int i = 0;
    while (i < strlen(ALPHABET))
    {
        if (ALPHABET[i] == key)
            return i;
        i++;
    }
    return -1;
}

static void trans_cleanup_module(void)
{
    dev_t dev0 = MKDEV(trans0.major, trans0.minor);

    kfree(trans0.transPuffer);
    kfree(trans1.transPuffer);

    /* cleanup_module is never called if registering failed */
    cdev_del(&trans0.cdev);
    cdev_del(&trans1.cdev);
    unregister_chrdev_region(dev0, 2); // Anzahl der zu löschenden device nummern
    // unregister_chrdev_region(dev1, 1);
    printk(KERN_ALERT "trans %d.%d and %d.%d deinstalled", trans0.major, trans0.minor, trans1.major, trans1.minor);
}

module_init(trans_init_module);    // Function that will be called for loading trans module
module_exit(trans_cleanup_module); // Function that will be called for removing trans module

// EOF