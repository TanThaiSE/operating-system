#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/random.h> ///< Can cho ham get_random_bytes

#define DEVICE_NAME     "RandomMachine"
#define CLASS_NAME      "Random"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thai Nhat Tan");
MODULE_DESCRIPTION("Section 1 - random number");
MODULE_VERSION("0.1");

static int numberOpens = 0; //Number of time that devide is opened
static int majorNumber; //device number 
static char numberGen[5]; //Number that module generate
static struct class* randClass = NULL;
static struct device* randDevice = NULL;

static int my_open(struct inode *, struct file *); // Called when device is opened
static int my_release(struct inode *, struct file *); // Called when device is closed
static ssize_t my_read(struct file *, char* , size_t, loff_t *); //Data send form device to user space

static struct file_operations fops = 
{
    .open = my_open,
    .read = my_read,
    .release = my_release,
};

//init function
static int __init CharDev_init(void){
    printk(KERN_INFO "Init the randDevice\n");

    //Cap phat major number cho device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0){
        printk(KERN_ALERT "failed to register a major number\n");
        return majorNumber;
    }

    //Dang ky device class
    randClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(randClass)){ //Check for error
        unregister_chrdev(majorNumber, DEVICE_NAME); //huy dky
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(randClass);
    }

    //Dang ky device driver 
    randDevice = device_create(randClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(randDevice)){
        class_destroy(randClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(randDevice);
    }
    printk(KERN_INFO "RANDOMMACHINE: device class created oke\n");
    return 0;
}

static void __exit CharDev_exit(void){
    device_destroy(randClass, MKDEV(majorNumber, 0)); //remove device
    class_unregister(randClass); //unregister device
    class_destroy(randClass);    //remove class 
    unregister_chrdev(majorNumber, DEVICE_NAME); //unregister major number
}

static int my_open(struct inode *i, struct file *f){
    numberOpens++;
    printk(KERN_INFO "RANDOMMACHINE: Device has been open %d time(s)\n", numberOpens);
    return 0;
}

static int my_release(struct inode *i, struct file *f){
   printk(KERN_INFO "Closed RANDOMMACHINE device successfully \n");
    return 0;
}


static ssize_t my_read(struct file *f, char* buffer, size_t len, loff_t *offset){
    int x = 0;
    
    get_random_bytes(numberGen, sizeof(int)); //Generate number

    x = copy_to_user(buffer, numberGen, sizeof(int));

    if (x == 0){
        printk(KERN_INFO " Success to sent\n");
        return 0;
    }
    else {
        printk(KERN_INFO "Failed to sent\n");
        return -EFAULT;
    }
}

module_init(CharDev_init);
module_exit(CharDev_exit);