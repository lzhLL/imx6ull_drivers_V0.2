#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <asm/uaccess.h>


#define     FIRST_CHRDEV_MAJOR                  (100)                 //主设备号
#define     FIRST_CHRDEV_NAME                   "first_chrdev"      //申请设备的设备名
#define     FIRST_CHRDEV_VERSION                "first_chrdev_V0.3"

static int first_chrdev_open(struct inode *inode, struct file *file)
{
    // printk("first_chrdev_open\r\n");
    return 0;
}


static ssize_t first_chrdev_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    // printk("first_chrdev_read\r\n");

    copy_to_user(buf, FIRST_CHRDEV_VERSION, strlen(FIRST_CHRDEV_VERSION));


    return 0;
}


static ssize_t first_chrdev_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    char write_buf[128] = {0};
    // printk("first_chrdev_write\r\n");
    copy_from_user(write_buf, buf, strlen(buf));

    printk("kernel recive data from user:%s\r\n", write_buf);

    return 0;
}


static int first_chrdev_release(struct inode *inode, struct file *file)
{
    // printk("first_chrdev_release\r\n");
    return 0;
}


static struct file_operations first_chrdev_fops = {
	.owner		= THIS_MODULE,
	.open		= first_chrdev_open,
	.read		= first_chrdev_read,
	.write		= first_chrdev_write,
	.release	= first_chrdev_release,
};


static int __init first_chrdev_init(void)
{
    int ret = 0;
    printk("first_chrdev_init\r\n");
    ret = register_chrdev(FIRST_CHRDEV_MAJOR, FIRST_CHRDEV_NAME, &first_chrdev_fops);       //注册设备驱动
	if(ret < 0) {
		printk("first_chrdev_init failed!\r\n");
	}	
    return 0;
}


static void __exit first_chrdev_exit(void)
{
	printk("first_chrdev_exit\r\n");
    unregister_chrdev(FIRST_CHRDEV_MAJOR, FIRST_CHRDEV_NAME);       //卸载字符设备驱动
    return ;
}


module_init(first_chrdev_init);     //驱动入口
module_exit(first_chrdev_exit);     //驱动出口

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lizh - 458279060@qq.com");



