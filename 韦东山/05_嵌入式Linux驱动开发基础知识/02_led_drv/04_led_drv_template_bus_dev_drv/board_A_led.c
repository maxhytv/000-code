
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

#include "led_resource.h"


static void led_dev_release(struct device *dev)
{
}


//- start：表示资源的起始地址。
//- flags：表示资源的属性标志，可以是以下值之一：
//  - IORESOURCE_MEM：表示内存资源。
//  - IORESOURCE_IO：表示I/O资源。
//  - IORESOURCE_IRQ：表示中断资源。
//  - IORESOURCE_DMA：表示DMA资源。
//- name：表示资源的名称。


//在驱动程序的probe函数中，可以通过调用platform_get_resource函数
//来获取这些资源，并使用它们进行设备的初始化和配置。
//例如，可以使用资源的start字段来获取资源的起始地址，
//并将其保存到相应的变量中，以便后续操作使用。

static struct resource resources[] = {
        {
                .start = GROUP_PIN(3,1),
                .flags = IORESOURCE_IRQ,
                .name = "100ask_led_pin",
        },
        {
                .start = GROUP_PIN(5,8),
                .flags = IORESOURCE_IRQ,
                .name = "100ask_led_pin",
        },
};


//- name：设备的名称，此处为"100ask_led"。
//- num_resources：设备需要的资源的数量，使用了一个名为ARRAY_SIZE的宏来计算resources数组的长度。
//- resource：设备需要的资源，此处为一个名为resources的数组。

//- dev：该结构体包含了一个名为dev的成员，
//是一个struct device类型的结构体变量，表示设备的设备节点。
//在这里，它只包含了一个名为release的函数指针，
//指向一个名为led_dev_release的函数，表示当设备节点被释放时将调用该函数。
//
//这段代码的作用是定义一个名为board_A_led_dev的设备，
//并将其与一个名为led_dev_release的函数关联起来，以在设备节点释放时调用该函数。
static struct platform_device board_A_led_dev = {
        .name = "100ask_led",
        .num_resources = ARRAY_SIZE(resources),
        .resource = resources,

//.dev是一个platform_device类型的结构体成员，
//它用于指定设备的相关信息，包括设备名称、设备属性、设备驱动等等。
//在这里，.dev被设置为一个包含.release成员的结构体，
//表示该设备的释放函数是led_dev_release。


       .dev = {
//.release是一个platform_device结构体中的成员函数，
//用于释放设备占用的资源。在设备不再需要使用时，
//设备驱动程序会调用.release函数来释放设备所占用的资源，
//以便其他设备或者系统可以使用这些资源。

//在这里，.dev被设置为一个包含.release成员的结构体，
//表示该设备的释放函数是led_dev_release。
//这意味着当该设备不再需要使用时，设备驱动程序会调用led_dev_release函数来释放设备所占用的资源，以便其他设备或者系统可以使用这些资源。

                .release = led_dev_release,
         },
};

//使用platform_device_register函数来注册一个平台设备，
//该设备是一个虚拟设备，用于模拟一个LED灯的控制。
//在驱动程序中，通过访问GPIO引脚来控制LED灯的亮灭。
//这个设备不需要注册字符设备驱动，因为它不需要提供类似于文件操作的接口。
static int __init led_dev_init(void)
{
    int err;
    
    err = platform_device_register(&board_A_led_dev);   
    
    return 0;
}

static void __exit led_dev_exit(void)
{
    platform_device_unregister(&board_A_led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");

