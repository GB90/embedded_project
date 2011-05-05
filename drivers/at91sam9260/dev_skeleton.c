/*********************************************************************************
 *  Copyright(c)  2011,  GuoWenxue<guowenxue@gmail.com>.
 *  All ringhts reserved.
 *
 *     Filename:  dev_skeleton.c
 *  Description:  Linux kernel common char device skeleton driver.
 *
 *        Usage:
 *                /tmp >: insmod dev_skeleton.ko  dev_major=218 debug=1
 *                /tmp >: dmesg
 *                Char skeleton driver version 1.0.0 initiliazed
 *                /tmp >: cat /proc/devices | grep skeleton    
 *                218 skeleton
 *                /tmp >: mknod -m 777 /dev/skeleton c 218 09
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "GuoWenxue<guowenxue@gmail.com>"
#define DRV_DESC                  "Linux kernel commone charactor device skeleton driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  SKELETON_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

struct cdev *skeleton_cdev = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

#define dbg_print(format,args...) if(DISABLE!=debug){printk(format, ##args);}

static int skeleton_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int skeleton_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int skeleton_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
                          unsigned long arg)
{
    //int   ret_val;
    switch (cmd)
    {
      case SET_DRV_DEBUG:
          printk("%s %s driver debug now.\n", DISABLE==arg ? "Disable" : "Enable", DEV_NAME);

          if (0 == arg)
              debug = DISABLE;
          else
              debug = ENABLE;

          break;

      case GET_DRV_VER:
          print_version(DRV_VERSION);
          return DRV_VERSION;
          break;

      default:
          printk("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
          return -ENOTTY;
    }

    return 0;
}

static struct file_operations skeleton_fops = {
    .owner = THIS_MODULE,
    .open = skeleton_open,
    .release = skeleton_release,
    .ioctl = skeleton_ioctl,
};

#ifdef CONFIG_PROC_FS
int read_proc_ioctl(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    if(offset > 0)
    {
        *eof = 1 ; 
        return 0;
    }

    len += sprintf(buf+len,"===============================================:\n");
    len += sprintf(buf+len,"All driver common ioctl:\n");
    len += sprintf(buf+len,"Enable driver debug: %u\n", SET_DRV_DEBUG);
    len += sprintf(buf+len,"Get driver version: %u\n", GET_DRV_VER);
    len += sprintf(buf+len,"\n");

    len += sprintf(buf+len,"Beep driver ioctl:\n");
    len += sprintf(buf+len,"Alarm enable: %u\n", BEEP_DISALARM);
    len += sprintf(buf+len,"Alarm disable: %u\n", BEEP_ENALARM);
    len += sprintf(buf+len,"\n");

    return len;
}
#endif

static struct class * dev_class;

static void skeleton_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    device_destroy (dev_class, devno);
    class_destroy (dev_class);

    cdev_del(skeleton_cdev);
    unregister_chrdev_region(devno, 1);

#ifdef CONFIG_PROC_FS
    remove_proc_entry("ioctl" , NULL);
#endif

    printk("%s driver removed\n", DEV_NAME);
}

static int __init skeleton_init(void)
{
    int result;
    dev_t devno;

    /*Alloc for the device for driver */
    if (0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, 1, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /*Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    /*Alloc cdev structure */
    skeleton_cdev = cdev_alloc();;
    if (NULL == skeleton_cdev)
    {
        printk("%s driver can't alloc for skeleton_cdev\n", DEV_NAME);
        goto ERROR;
    }

    /*Initialize cdev structure and register it */
    skeleton_cdev->owner = THIS_MODULE;
    skeleton_cdev->ops = &skeleton_fops;
    result = cdev_add(skeleton_cdev, devno, 1);
    if (0 != result)
    {
        printk("%s driver can't alloc for skeleton_cdev\n", DEV_NAME);
        goto ERROR;
    }

    /*Create device /dev/$DEV_NAME */
    dev_class = class_create (THIS_MODULE, DEV_NAME);
    if(dev_class)
        device_create (dev_class, NULL, devno, "%s", DEV_NAME); 
    else
        printk ("%s driver can't create device class\n", DEV_NAME);    

#ifdef CONFIG_PROC_FS 
    create_proc_read_entry("ioctl", 0, NULL, read_proc_ioctl, NULL);
#endif

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
           DRV_REVER_VER);

    return 0;

  ERROR:
    skeleton_cleanup();
    return -ENODEV;

}

module_init(skeleton_init);
module_exit(skeleton_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
