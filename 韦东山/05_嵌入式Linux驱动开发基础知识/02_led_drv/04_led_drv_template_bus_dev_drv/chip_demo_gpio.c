#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>

#include "led_opr.h"
#include "leddrv.h"
#include "led_resource.h"

static int g_ledpins[100];
static int g_ledcnt = 0;

static int board_demo_led_init (int which) /* 初始化LED, which-哪个LED */       
{   
    //printk("%s %s line %d, led %d\n", __FILE__, __FUNCTION__, __LINE__, which);
    
    printk("init gpio: group %d, pin %d\n", GROUP(g_ledpins[which]), PIN(g_ledpins[which]));
    switch(GROUP(g_ledpins[which]))
    {
        case 0:
        {
            printk("init pin of group 0 ...\n");
            break;
        }
        case 1:
        {
            printk("init pin of group 1 ...\n");
            break;
        }
        case 2:
        {
            printk("init pin of group 2 ...\n");
            break;
        }
        case 3:
        {
            printk("init pin of group 3 ...\n");
            break;
        }
    }
    
    return 0;
}

static int board_demo_led_ctl (int which, char status) /* 控制LED, which-哪个LED, status:1-亮,0-灭 */
{
    //printk("%s %s line %d, led %d, %s\n", __FILE__, __FUNCTION__, __LINE__, which, status ? "on" : "off");
    printk("set led %s: group %d, pin %d\n", status ? "on" : "off", GROUP(g_ledpins[which]), PIN(g_ledpins[which]));

    switch(GROUP(g_ledpins[which]))
    {
        case 0:
        {
            printk("set pin of group 0 ...\n");
            break;
        }
        case 1:
        {
            printk("set pin of group 1 ...\n");
            break;
        }
        case 2:
        {
            printk("set pin of group 2 ...\n");
            break;
        }
        case 3:
        {
            printk("set pin of group 3 ...\n");
            break;
        }
    }

    return 0;
}

static struct led_operations board_demo_led_opr = {
    .init = board_demo_led_init,
    .ctl  = board_demo_led_ctl,
};

struct led_operations *get_board_led_opr(void)
{
    return &board_demo_led_opr;
}




//这段代码是驱动程序的probe函数，用于在设备被系统检测到时初始化设备并进行必要的配置。
//这个函数接受一个指向platform_device类型的指针作为参数，该指针指向被检测到的设备对象。

//首先定义了一个指向resource类型的指针res，用于获取设备的资源。
//然后定义了一个整型变量i，用于遍历设备的资源列表。

//接下来是一个while循环，它的条件是1，即无限循环。


//接着，调用led_class_create_device函数创建设备节点，
//其中参数是全局变量g_ledcnt，表示设备节点的编号。
//该函数的作用是在/sys/class/leds目录下创建一个名为ledX的设备节点，
//其中X是设备节点的编号。

//最后，将设备节点的编号g_ledcnt自增1，继续下一次循环，直到遍历完所有的资源。


static int chip_demo_gpio_probe(struct platform_device *pdev)
{
    struct resource *res;
    int i = 0;

    while (1)
    {
//在循环中，通过调用platform_get_resource函数获取设备资源，
//其中第一个参数是指向设备对象的指针，第二个参数是资源类型，
//这里是IORESOURCE_IRQ，表示获取中断资源。第三个参数是资源的索引，每次循环i自增1。

//如果获取到的资源为NULL，说明已经遍历完了所有的资源，此时退出循环。
//否则，将资源的起始地址存储到一个全局数组g_ledpins中，以便后续操作使用。
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i++);
        if (!res)
            break;
        
        g_ledpins[g_ledcnt] = res->start;
        led_class_create_device(g_ledcnt);
        g_ledcnt++;
    }
    //循环结束后，返回0表示设备初始化成功。
    //这样，当系统检测到该设备时，就会调用probe函数，对设备进行初始化和配置，
    //为后续操作做好准备。
    return 0;
    
}
用于注销设备时释放平台设备中的资源。具体来说，它使用platform_get_resource函数来获取设备的资源信息，然后依次遍历每个资源并释放它们。
//用于注销设备时释放平台设备中的资源。
//具体来说，它使用platform_get_resource函数来获取设备的资源信息，
//然后依次遍历每个资源并释放它们。
static int chip_demo_gpio_remove(struct platform_device *pdev)
{
    struct resource *res;
    int i = 0;

    while (1)
    {
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
        if (!res)
            break;
        
        led_class_destroy_device(i);
        i++;
        g_ledcnt--;
    }
    return 0;
}


//- probe：用于检测该设备的函数指针，当系统检测到该设备时，将调用该函数。
//- remove：用于卸载该设备的函数指针，当设备被卸载时，将调用该函数。
//- driver：该结构体包含了一个名为driver的成员，是一个struct device_driver类型的结构体变量，表示设备的驱动程序。在这里，它只包含了一个名为name的字符串变量，用于标识驱动程序的名称，此处为"100ask_led"。

//这段代码的作用是定义一个名为chip_demo_gpio_driver的驱动程序，
//该驱动程序包含了probe和remove函数指针，用于检测和卸载设备，以及一个名称为"100ask_led"的驱动程序名称。
static struct platform_driver chip_demo_gpio_driver = {
    .probe      = chip_demo_gpio_probe,
    .remove     = chip_demo_gpio_remove,
    .driver     = {
        .name   = "100ask_led",-
    },
};

static int __init chip_demo_gpio_drv_init(void)
{
    int err;
    
    err = platform_driver_register(&chip_demo_gpio_driver); 
    register_led_operations(&board_demo_led_opr);
    
    return 0;
}

static void __exit lchip_demo_gpio_drv_exit(void)
{
    platform_driver_unregister(&chip_demo_gpio_driver);
}

module_init(chip_demo_gpio_drv_init);
module_exit(lchip_demo_gpio_drv_exit);

MODULE_LICENSE("GPL");

