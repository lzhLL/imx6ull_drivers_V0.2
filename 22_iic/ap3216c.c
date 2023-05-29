#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include "ap3216creg.h"

#define     AP3216C_CNT     1
#define     AP3216C_NAME    "ap3216c"


typedef struct ap3216c_dev {
    int major;
    int minor;
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
}ap3216c_dev_t;


static struct ap3216c_dev ap3216cdev;

static int ap3216c_open(struct inode *inode, struct file *filp)
{
    printk("ap3216c_open\r\n");
    filp->private_data = &ap3216cdev;
    return 0;
}


static int ap3216c_release(struct inode *inode, struct file *filp)
{
    
    struct ap3216c_dev* dev = (struct ap3216c_dev*)filp->private_data;
    printk("ap3216c_release\r\n");
    return 0;
}


ssize_t ap3216c_read(struct file *file, char __user *buf, size_t cnt, loff_t *off)
{
    printk("ap3216c_read\r\n");

    return 0;
}



/* 字符设备操作集 */
static const struct file_operations ap3216c_fops = {
	.owner	= THIS_MODULE,
	.open	= ap3216c_open,
    .read   = ap3216c_read,
	.release= ap3216c_release,
};


static int ap3216c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    printk("ap3216c_probe!\r\n");


    /* 搭建字符设备驱动框架，在/dev/ */
	//2.注册字符设备
    if (ap3216cdev.major)    //如果给定主设备号
    {
        ap3216cdev.devid = MKDEV(ap3216cdev.major, 0);
        ret = register_chrdev_region(ap3216cdev.devid, AP3216C_CNT, AP3216C_NAME); 
    }
    else        /* 如果没给定主设备号 */
    {
        ret = alloc_chrdev_region(&ap3216cdev.devid, 0, AP3216C_CNT, AP3216C_NAME);
        ap3216cdev.major = MAJOR(ap3216cdev.devid);
        ap3216cdev.minor = MINOR(ap3216cdev.devid);
    }
    
    if (ret < 0)
    {
        printk("ap3216cdev chrdev_region err");
        goto fail_devid;
    }
    printk("ap3216cdev major=%d, minor = %d\r\n", ap3216cdev.major, ap3216cdev.minor);

    /* 3.注册字符设备 */
    ap3216cdev.cdev.owner = THIS_MODULE;
    cdev_init(&ap3216cdev.cdev, &ap3216c_fops);

    ret = cdev_add(&ap3216cdev.cdev, ap3216cdev.devid, AP3216C_CNT);
    if (ret < 0)
    {
        goto fail_cdev;
    }
    

    /* 4.自动创建设备节点 */
    ap3216cdev.class = class_create(THIS_MODULE, AP3216C_NAME);
    if (IS_ERR(ap3216cdev.class))
    {
        ret = PTR_ERR(ap3216cdev.class);
        goto fail_class;
    }
    
    ap3216cdev.device = device_create(ap3216cdev.class, NULL, ap3216cdev.devid, NULL, AP3216C_NAME);
    if (IS_ERR(ap3216cdev.device))
    {
        ret = PTR_ERR(ap3216cdev.device);
        goto fail_device;
    }


    return 0;

fail_device:
    device_destroy(ap3216cdev.class, ap3216cdev.devid);
fail_class:
    cdev_del(&ap3216cdev.cdev);
fail_cdev:
    unregister_chrdev_region(ap3216cdev.devid, AP3216C_CNT);
fail_devid:
    return ret;



}


static int ap3216c_remove(struct i2c_client *client)
{
    /* 删除字符设备 */
    cdev_del(&ap3216cdev.cdev);

	//卸载字符设备
	unregister_chrdev_region(ap3216cdev.devid, AP3216C_CNT);
	
    //先摧毁设备
    device_destroy(ap3216cdev.class, ap3216cdev.devid);

    //摧毁类
    class_destroy(ap3216cdev.class);

    return 0;
}



/* 传统的匹配表 */
static struct i2c_device_id ap3216c_id[] = {
    {"alientek,ap3216c", 0},
    {}

};

/* 设备树匹配表 */
static struct of_device_id ap3216c_of_match[] = {
    {.compatible = "alientek,ap3216c"},
    {}
};

/* i2c_driver */
static struct i2c_driver ap3216c_driver = {
    .probe = ap3216c_probe,
    .remove = ap3216c_remove,
    .driver = {
        .name = "ap3216c",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(ap3216c_of_match),
    },
    .id_table = ap3216c_id,
}; 


/* 驱动入口函数 */
static int __init ap3216c_init(void)
{
    int ret = 0;

    ret = i2c_add_driver(&ap3216c_driver);

    return ret;
}


/* 驱动出口函数 */
static void __exit ap3216c_exit(void)
{
    i2c_del_driver(&ap3216c_driver);

    return ;
}


module_init(ap3216c_init);
module_exit(ap3216c_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lizh");




