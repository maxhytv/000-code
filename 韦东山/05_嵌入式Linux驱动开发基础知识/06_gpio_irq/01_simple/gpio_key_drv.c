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
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>


struct gpio_key{
	int gpio;
	struct gpio_desc *gpiod;
	int flag;
	int irq;
} ;

static struct gpio_key *gpio_keys_100ask;

//定义了一个名为gpio_key_isr的静态中断服务函数，
//该函数将接收作为参数的IRQ号和设备标识符dev_id，
//并返回一个irqreturn_t类型的值。
static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = dev_id;
	int val;

	//读取输入值，并将其赋值给变量val。
	//gpiod_get_value函数用于获取指定GPIO的值，
	//它的参数是表示GPIO设备的gpiod结构体的指针。
	//读取成功时，该函数将返回0或1的整数类型数据，
	//分别表示GPIO的低电平和高电平状态。
	val = gpiod_get_value(gpio_key->gpiod);
	

	printk("key %d %d\n", gpio_key->gpio, val);
	
	return IRQ_HANDLED;
}

/* 1. 从platform_device获得GPIO
 * 2. gpio=>irq
 * 3. request_irq
 */
static int gpio_key_probe(struct platform_device *pdev)
{
	int err;
	struct device_node *node = pdev->dev.of_node;
	int count;
	int i;
//被用来表示GPIO在设备树中的属性信息。
//通过枚举类型，我们可以更方便地定义GPIO的输入输出属性
//（如上拉、下拉、高电平、低电平等），
//以及支持的特殊功能，如中断和SPI等。
//OF_GPIO_ACTIVE_LOW = 0x1, /* 表示 GPIO 是低电平有效 */
	enum of_gpio_flags flag;
	unsigned flags = GPIOF_IN;
		
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
//该函数用于计算设备树中指定节点下可用的GPIO数量。
	count = of_gpio_count(node);
	if (!count)
	{
		printk("%s %s line %d, there isn't any gpio available\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
//这行代码用于分配内存，以存储GPIO键的信息。
//该函数使用kzalloc函数，其作用类似于标准的C库函数malloc。
//这里我们需要分配一个结构体数组，
//并将其大小设置为count个struct gpio_key结构体的大小。
//通过这种方式，我们可以在内存中创建足够的空间，
//以存储每个GPIO键的信息，包括其所连接的GPIO口号和属性信息。
	gpio_keys_100ask = kzalloc(sizeof(struct gpio_key) * count, GFP_KERNEL);
	
	for (i = 0; i < count; i++)
	{
		gpio_keys_100ask[i].gpio = of_get_gpio_flags(node, i, &flag);
		if (gpio_keys_100ask[i].gpio < 0)
		{
			printk("%s %s line %d, of_get_gpio_flags fail\n", __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
		gpio_keys_100ask[i].gpiod = gpio_to_desc(gpio_keys_100ask[i].gpio);
		gpio_keys_100ask[i].flag = flag & OF_GPIO_ACTIVE_LOW;

		if (flag & OF_GPIO_ACTIVE_LOW)
			flags |= GPIOF_ACTIVE_LOW;
//求一个GPIO引脚，并在设备移除时自动释放
		err = devm_gpio_request_one(&pdev->dev, gpio_keys_100ask[i].gpio, flags, NULL);

		
		gpio_keys_100ask[i].irq  = gpio_to_irq(gpio_keys_100ask[i].gpio);
	}

	for (i = 0; i < count; i++)
	{
		err = request_irq(gpio_keys_100ask[i].irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "100ask_gpio_key", &gpio_keys_100ask[i]);
	}
        
    return 0;
    
}

static int gpio_key_remove(struct platform_device *pdev)
{
	//int err;
	struct device_node *node = pdev->dev.of_node;
	int count;
	int i;

	count = of_gpio_count(node);
	for (i = 0; i < count; i++)
	{
		free_irq(gpio_keys_100ask[i].irq, &gpio_keys_100ask[i]);
	}
	kfree(gpio_keys_100ask);
    return 0;
}


static const struct of_device_id ask100_keys[] = {
    { .compatible = "100ask,gpio_key" },
    { },
};

/* 1. 定义platform_driver */
static struct platform_driver gpio_keys_driver = {
    .probe      = gpio_key_probe,
    .remove     = gpio_key_remove,
    .driver     = {
        .name   = "100ask_gpio_key",
        .of_match_table = ask100_keys,
    },
};

/* 2. 在入口函数注册platform_driver */
static int __init gpio_key_init(void)
{
    int err;
    
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	
    err = platform_driver_register(&gpio_keys_driver); 
	
	return err;
}

/* 3. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数
 *     卸载platform_driver
 */
static void __exit gpio_key_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&gpio_keys_driver);
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(gpio_key_init);
module_exit(gpio_key_exit);

MODULE_LICENSE("GPL");


