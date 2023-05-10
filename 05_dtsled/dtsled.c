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
#include <linux/of_device.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>

#define     DTSLED_CNT          1               /* 设备号个数 */
#define     DTSLED_NAME       "dtsled"          /* 名字  */
#define     LEDOFF              0               /* led关闭 */
#define     LEDON               1               /* led打开 */

/* 地址映射后的虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;


/* dtsled设备结构体 */
struct dtsled_dev{
    dev_t devid;            /* 设备号 */
    struct cdev cdev;       /* 字符设备结构体 */
    struct class *class;    /* 设备类结构体 */
    struct device *device;  /* 设备结构体 */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    struct device_node *nd; /* 设备树节点 */
};

struct dtsled_dev dtsled;       /* led设备 */


/* LED灯打开/关闭 */
static void led_switch(u8 state)
{
    u32 val;
    if (state == LEDON)
    {
        /* 灯打开  */
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);            /* bit3置0,打开LED灯 */
        writel(val, GPIO1_DR);        
    }
    else
    {
        /* 灯关闭 */
        val = readl(GPIO1_DR);
        val |= (1 << 3);            /* bit3置1关闭LED灯 */
        writel(val, GPIO1_DR);        
    }

    return ;
}

static int dtsled_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &dtsled;
    return 0;
}


static int dtsled_release(struct inode *inode, struct file *filp)
{
    struct dtsled_dev *dev = (struct dtsled_dev*)filp->private_data;

    return 0;
}


static ssize_t dtsled_write(struct file * filp, const char __user* buf, size_t count, loff_t* ppos)
{
    struct dtsled_dev *dev = (struct dtsled_dev*)filp->private_data;
    int ret = 0;
    unsigned char data_buf[1] = {0};

    ret = copy_from_user(data_buf, buf, count);
    if (ret < 0)
    {
        return -EFAULT; 
    }

    led_switch(data_buf[0]);    //判断灯开关
    if (data_buf[0])
    {
        printk("LED ON!\r\n");
    }
    else
    {
        printk("LED OFF!\r\n");
    }

    return 0;
}

/* dtsled字符设备操作集 */
static const struct file_operations dtsled_fops = {
    .owner = THIS_MODULE,
    .write = dtsled_write,
    .open = dtsled_open,
    .release = dtsled_release,

};


/* 入口 */ 
static int __init dtsled_init(void)
{
    int ret = -1;
    const char * str = NULL;
    u32 reg_data[10] = {0};
    u8 i = 0;
    u32 val = 0;

    /* 注册字符设备 */
    /* 1.申请设备号 */
    dtsled.major = 0;       /* 设备号由内核分配 */
    if(dtsled.major) {      /* 定义了设备号 */
        dtsled.devid = MKDEV(dtsled.major, 0);
        ret = register_chrdev_region(dtsled.devid, DTSLED_CNT, DTSLED_NAME);
    } else {                /* 没有给定设备号 */
        ret = alloc_chrdev_region(&dtsled.devid, 0, DTSLED_CNT, DTSLED_NAME);  
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.devid);
    }

    if(ret < 0) {
        goto fail_devid;
    }

    /* 2.添加字符设备 */
    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &dtsled_fops);
    ret = cdev_add(&dtsled.cdev, dtsled.devid, DTSLED_CNT);
    if (ret < 0) {
        goto fail_cdev;
    }
    
    /* 3.自动创建设备节点 */
    dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
    if (IS_ERR(dtsled.class)) {
        ret = PTR_ERR(dtsled.class);
        goto fail_class;
    }
            
    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if ( IS_ERR(dtsled.device) ) {
        ret = PTR_ERR(dtsled.device);
        goto fail_device;
    }
    
    /* 获取设备树属性内容 */
    dtsled.nd = of_find_node_by_path("/alphaled");
    if (dtsled.nd == NULL) {
        ret = -EINVAL;
        goto fail_findnd;
    }

    /* 获取属性 */
    ret = of_property_read_string(dtsled.nd, "status", &str);
    if (ret < 0) {
        goto fail_rs;
    } else {
        printk("status %s\r\n", str);
    }


    ret = of_property_read_string(dtsled.nd, "compatible", &str);
    if (ret < 0) {
        goto fail_rs;
    } else {
        printk("compatible %s\r\n", str);
    }
    
#if 0
    /* 获取reg内容 */
    ret = of_property_read_u32_array(dtsled.nd, "reg", reg_data, 10);
    if(ret < 0) {
        goto fail_rs;
    } else {
        printk("reg data:\r\n");
        for(i = 0; i < 10; i++) {
            printk("%#X ", *(reg_data+i));
        }
        printk("\r\n");
    }

    /* LED初始化 */
	/* 地址映射 */

	IMX6U_CCM_CCGR1			= ioremap(reg_data[0], reg_data[1]);
	SW_MUX_GPIO1_IO03		= ioremap(reg_data[2], reg_data[3]);
	SW_PAD_GPIO1_IO03		= ioremap(reg_data[4], reg_data[5]);
	GPIO1_DR				= ioremap(reg_data[6], reg_data[7]);
	GPIO1_GDIR				= ioremap(reg_data[8], reg_data[9]);
#else
	IMX6U_CCM_CCGR1			= of_iomap(dtsled.nd, 0);
	SW_MUX_GPIO1_IO03		= of_iomap(dtsled.nd, 1);
	SW_PAD_GPIO1_IO03		= of_iomap(dtsled.nd, 2);
	GPIO1_DR				= of_iomap(dtsled.nd, 3);
	GPIO1_GDIR				= of_iomap(dtsled.nd, 0);
#endif

	/* 初始化寄存器 */
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26);  /* 先清除以前的配置bit26,27 */
    val |= 3 << 26;     /* bit26,27置1 */
    writel(val, IMX6U_CCM_CCGR1);
 
    writel(0x5, SW_MUX_GPIO1_IO03);     /* 设置复用 */
    writel(0X10B0, SW_PAD_GPIO1_IO03);  /* 设置电气属性 */

    val = readl(GPIO1_GDIR);
    val |= 1 << 3;              /* bit3置1,设置为输出 */
    writel(val, GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val |= (1 << 3);            /* bit3置1,关闭LED灯 */
    writel(val, GPIO1_DR);    


    return 0;

fail_rs:

fail_findnd:
    device_destroy(dtsled.class, dtsled.devid);

fail_device:
    class_destroy(dtsled.class);

fail_class:
    cdev_del(&dtsled.cdev);

fail_cdev:
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);

fail_devid:
    return ret;    
}


/* 出口 */
static void __exit dtsled_exit(void)
{
    unsigned int val = 0;

    val = readl(GPIO1_DR);
    val |= (1 << 3);            /* bit3置1,关闭LED灯 */
    writel(val, GPIO1_DR);

	/* 取消地址映射 */
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    /* 删除字符设备 */
    cdev_del(&dtsled.cdev);

    /* 释放设备号 */
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);

    /* 摧毁设备 */
    device_destroy(dtsled.class, dtsled.devid);

    /* 摧毁类 */
    class_destroy(dtsled.class);

    return ;
}


/* 注册驱和卸载驱动 */
module_init(dtsled_init);
module_exit(dtsled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lizh");

