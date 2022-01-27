#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define TRANS1_DEVICE_NAME "trans1"
#define TRANS0_DEVICE_NAME "trans0"

//TODO: Puffer welcher 40 Zeichen groß ist. 
//GFP_KERNEL-> Allocate normal kernel ram. may sleep
static char trans1_puffer = kmalloc(40, GFP_KERNEL);
static char trans0_puffer = kmalloc(40, GFP_KERNEL);

//first ist beginnende device nummer, 
int register_chrdev_region(dev_t first, unsigned int count, char *name);

printk(struct_size(trans1_puffer));

static int dev_open(struct inode*, struct file*);
//Alternativ auch close 
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int major;

static int __init rickroll_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        printk(KERN_ALERT "trans0 load failed\n");
        return major;
    }

    printk(KERN_INFO "trans0 module has been loaded: %d\n", major);
    return 0;
}

static void __exit rickroll_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "trans0 module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "Rickroll device opened\n");
   return 0;
}

//schreibender Zugriff auf trans0 verschlüsselt den Text und legt dieses auf FIFO Puffer ab
static ssize_t dev_write(struct file *filep, const char *buffer,
                         size_t len, loff_t *offset) {

   printk(KERN_INFO "Sorry, rickroll is read only\n");
   return -EFAULT;
}

static int dev_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "Rickroll device closed\n");
   return 0;
}

//lesender Zugriff auf trans0 gitb den verschlüsselten Text aus.
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int errors = 0;
    char *message = "never gonna give you up, never gonna let you down... ";
    int message_len = strlen(message);

    errors = copy_to_user(buffer, message, message_len);

    return errors == 0 ? message_len : -EFAULT;
}

module_init(rickroll_init);
module_exit(rickroll_exit);

char caeser_on(char key)    {
    switch(key) {
        case ' ': return 'c';
        case 'X': return ' ';
        case 'Y': return 'a';
        case 'Z': return 'b';
        case 'x': return 'A';
        case 'y': return 'B';
        case 'z': return 'C';
        default:
            //key >= '\65'
            if((key >= 65 && key <=87) || (key >= 97 && key <= 119))    {
                return key+3;
            }
    }   
}

char caeser_off(char key)   {
    switch(key) {
        case ' ': return 'X';
        case 'C': return 'z';
        case 'B': return 'y';
        case 'A': return 'x';
        case 'c': return ' ';
        case 'b': return 'Z';
        case 'a': return 'Y';
        default:
            //key >= '\65'
            if((key >= 68 && key <=90) || (key >= 100 && key <= 122))    {
                return key-3;
            }
    }   
}

