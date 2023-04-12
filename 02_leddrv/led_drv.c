#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define         LED_DRV_MAJOR               (200)
#define         LED_DRV_NAME                "led_drv"

/* 寄存器物理地址 */
#define CCM_CCGR1_BASE              		(0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE      		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE      		(0X020E02F4)
#define GPIO1_DR_BASE               		(0X0209C000)
#define GPIO1_GDIR_BASE             		(0X0209C004)

/* 地址映射后的虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;



#define     LEDOFF          0               /* led关闭 */
#define     LEDON           1               /* led打开 */

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


static int led_drv_open(struct inode *inode, struct file *file)
{
    return 0;
}


static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    int ret = 0;
    unsigned char data_buf[1] = {0};

    ret = copy_from_user(data_buf, buf, len);
    if (ret < 0)
    {
        printk("kernel write failed\r\n");
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


static int led_drv_release(struct inode *inode, struct file *file)
{
    return 0;
}


/* 字符设备操作集 */
static const struct file_operations led_drv_fops = {
	.owner	= THIS_MODULE,
	.write	= led_drv_write,
	.open	= led_drv_open,
	.release= led_drv_release,
};


/* 入口*/
static int __init led_drv_init(void)
{
	int ret = 0;
	unsigned int val = 0;

	/* 地址映射 */
	IMX6U_CCM_CCGR1			= ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03		= ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
	SW_PAD_GPIO1_IO03		= ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR				= ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR				= ioremap(GPIO1_GDIR_BASE, 4);


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

	//注册字符设备
    ret = register_chrdev(LED_DRV_MAJOR, LED_DRV_NAME, &led_drv_fops);
	if (ret < 0)
	{
		printk("register led_drv failed!\r\n");
		return -EIO;
	}
	printk("led_drv_init\r\n");

    return 0;
}

/* 出口*/
static void __exit led_drv_exit(void)
{
	unsigned int val = 0;
    printk("led_drv_exit\r\n");

    val = readl(GPIO1_DR);
    val |= (1 << 3);            /* bit3置1,关闭LED灯 */
    writel(val, GPIO1_DR);

	/* 取消地址映射 */
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

	//卸载字符设备
	unregister_chrdev(LED_DRV_MAJOR, LED_DRV_NAME);
	
    return ;
}


/* 注册驱动加载和卸载 */
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lizh");






