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
#include <linux/gpio/consumer.h>


/* 1. 确定主设备号                                                                 */
static int major = 0;
static struct class *led_class;

//gpio_desc（GPIO 描述符）包含了 GPIO 的各种属性，
//如引脚号、方向、中断、名称等，可以用于控制和管理 GPIO。
//在上下文中，led_gpio 可能是在驱动程序中用于控制 LED 灯的 GPIO 描述符。
//该指针变量可以通过调用 GPIO 管理库中的函数（如 gpiod_get 函数）获取，
//然后可以使用 GPIO 描述符中的其他函数
//（如 gpiod_set_value 函数）来控制 GPIO 的状态。
static struct gpio_desc *led_gpio;


/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* write(fd, &val, 1); */
static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);

	/* 根据次设备号和status控制LED */
		//用于设置指定 GPIO 设备的电平状态，
	gpiod_set_value(led_gpio, status);
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 根据次设备号初始化LED */
	//用于将 GPIO 设备设置为输出模式，
	//并设置初始输出电平。
	gpiod_direction_output(led_gpio, 0);
	
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* 定义自己的file_operations结构体                                              */
static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};

/* 4. 从platform_device获得GPIO
 *    把file_operations结构体告诉内核：注册驱动程序
 */
static int chip_demo_gpio_probe(struct platform_device *pdev)
{
	int err;
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	/* 4.1 设备树中定义有: led-gpios=<...>;	*/
	//该函数用于获取指定名称的 GPIO，
	//并返回一个指向该 GPIO 对象的指针 led_gpio。
	
	//&pdev->dev：指向 platform_device 结构体中的设备对象的指针。
	//该参数用于指定要获取 GPIO 的设备对象，
	//通常使用 platform_device 结构体中的 dev 成员变量。
	
	//"led"：要获取的 GPIO 的名称，
	//通常是在设备树中的节点名称或者在驱动程序中定义的名称。
	//0：GPIO 的标志位，通常为 0。
    led_gpio = gpiod_get(&pdev->dev, "led", 0);
	if (IS_ERR(led_gpio)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led_gpio);
	}
    
	/* 4.2 注册file_operations 	*/
	major = register_chrdev(0, "100ask_led", &led_drv);  /* /dev/led */

	led_class = class_create(THIS_MODULE, "100ask_led_class");
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "led");
		gpiod_free(led_gpio);
		return PTR_ERR(led_class);
	}

	device_create(led_class, NULL, MKDEV(major, 0), NULL, "100ask_led%d", 0); /* /dev/100ask_led0 */
        
    return 0;
    
}

static int chip_demo_gpio_remove(struct platform_device *pdev)
{
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "100ask_led");
	gpiod_free(led_gpio);
    
    return 0;
}

//用于设备树匹配的设备 ID 表（device ID table）。
//在 Linux 内核中，设备树可以通过设备 ID 表来匹配设备节点和驱动程序，
//从而自动加载相应的驱动程序。

//在上下文中，ask100_leds 是一个设备 ID 表，
//用于匹配名为 "100ask,leddrv" 的设备节点。
//该设备节点通常在设备树中描述了设备的类型、地址、中断、资源等信息。
static const struct of_device_id ask100_leds[] = {
    { .compatible = "100ask,leddrv" },
    { },
};

/* 1. 定义platform_driver */
static struct platform_driver chip_demo_gpio_driver = {
    .probe      = chip_demo_gpio_probe,
    .remove     = chip_demo_gpio_remove,
    .driver     = {
        .name   = "100ask_led",
        .of_match_table = ask100_leds,
    },
};

/* 2. 在入口函数注册platform_driver */
static int __init led_init(void)
{
    int err;
    
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

//用于将驱动程序注册到内核中，
//以便在内核启动时自动加载该驱动程序。
//如果注册成功，该函数将返回 0，否则返回一个负数错误码。

//在这里，该函数将 chip_demo_gpio_driver 结构体
//注册到内核中，从而使得内核能够自动加载该驱动程序，
///并建立设备节点和驱动程序之间的关联关系。
    err = platform_driver_register(&chip_demo_gpio_driver); 
	
	return err;
}

/* 3. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数
 *     卸载platform_driver
 */
static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&chip_demo_gpio_driver);
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


