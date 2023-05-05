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

#if 0
	backlight {
		compatible = "pwm-backlight";
		pwms = <&pwm1 0 5000000>;
		brightness-levels = <0 4 8 16 32 64 128 255>;
		default-brightness-level = <6>;
		status = "okay";
	};

    i2c1: i2c@021a0000 {
        #address-cells = <1>;
        #size-cells = <0>;
        compatible = "fsl,imx6sl-i2c", "fsl,imx21-i2c";
        reg = <0x021a0000 0x4000>;
        interrupts = <0 36 IRQ_TYPE_LEVEL_HIGH>;
        clocks = <&clks IMX6SL_CLK_I2C1>;
        status = "disabled";
    };

#endif



/*
 * 模块入口 
 */

#if 1
static int __init dtsof_init(void)
{
    int ret = 0;
    struct device_node *bl_nd = NULL;          /* i2c节点 */
    struct property *comppro = NULL;
    const char * str = NULL;
    u32 def_value = 0;
    u32* brival = NULL;
    u32 elemsize = 0;
    u8 i = 0;

    /* 1、找到i2c1节点，路径是：/soc/aips-bus@02100000/i2c@021a0000 */
	bl_nd = of_find_node_by_path("/soc/aips-bus@02100000/i2c@021a0000");
	if (bl_nd == NULL) {
        ret = -EINVAL;
        goto fail_findnd;
	} else {
        printk("find i2c1 node success!\r\n");
    }

    /* 2、获取compatible属性 */
    comppro =  of_find_property(bl_nd, "compatible", NULL);    
    if (comppro == NULL) {
        ret = -EINVAL;
        goto fail_finpro;
    } else {
        printk("compatible = %s\r\n", (char*)comppro->value);
    }


    return ret;
    
fail_finpro:
fail_findnd:
    return ret;    
}
#else
static int __init dtsof_init(void)
{
    int ret = 0;
    struct device_node *bl_nd = NULL;          /* 节点 */
    struct property *comppro = NULL;
    const char * str = NULL;
    u32 def_value = 0;
    u32* brival = NULL;
    u32 elemsize = 0;
    u8 i = 0;


    /* 1、找到backlight节点，路径是：/backlight */
	bl_nd = of_find_node_by_path("/backlight");
	if (bl_nd == NULL) {
        ret = -EINVAL;
        goto fail_findnd;

	}

    /* 2、获取compatible属性 */
    comppro =  of_find_property(bl_nd, "compatible", NULL);    
    if (comppro == NULL) {
        ret = -EINVAL;
        goto fail_finpro;
    } else {
        printk("compatible = %s\r\n", (char*)comppro->value);
    }

    ret = of_property_read_string(bl_nd, "status", &str);
    if (ret < 0)
    {
        goto fail_rs;
    } else {
        printk("status %s\r\n", str);
    }
    
    /* 3、获取数字属性值 */
    ret = of_property_read_u32(bl_nd, "default-brightness-level", &def_value);
    if(ret < 0) {
        goto fail_read32;
    } else {
        printk("default-brightness-level = %d\r\n", def_value);
    }

    /* 4、获取数组类型的属性 */
    elemsize = of_property_count_elems_of_size(bl_nd, "brightness-levels", sizeof(u32));
    if(elemsize < 0) {
        ret = elemsize;
        goto fail_readele;
    } else {
        printk("brightness-levels elems size= %d\r\n", elemsize);
    }


    /* 申请内存 */
    brival = kmalloc(elemsize * sizeof(u32), GFP_KERNEL);
    if(!brival) {
        ret = -EINVAL;
        goto fail_mem;
    }

    /* 获取数组内容 */
    ret = of_property_read_u32_array(bl_nd, "brightness-levels", brival, elemsize);
    if(ret < 0) {
        goto fail_read32array;
    } else {
        for(i = 0; i < elemsize; i++) {
            printk("brightness-levels[%d]:%d\r\n", i, *(brival+i));
        }
        kfree(brival);      /* 释放内存 */
    }
    return 0;


fail_read32array:
    kfree(brival);      /* 释放内存 */
fail_mem:
fail_readele:
fail_read32:
fail_rs:
fail_finpro:
fail_findnd:
    return ret;
}
#endif

/*
 *模块出口 
*/
static void __exit dtsof_exit(void)
{


    return ;        
}





/* 模块入口和出口 */
module_init(dtsof_init);
module_exit(dtsof_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lizh");
