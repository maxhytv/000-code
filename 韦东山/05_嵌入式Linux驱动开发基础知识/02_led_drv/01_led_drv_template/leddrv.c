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

#include "led_opr.h"

#define LED_NUM 2

/* 1. 确定主设备号                                                                 */
static int major = 0;
static struct class *led_class;
struct led_operations *p_led_opr;


#define MIN(a, b) (a < b ? a : b)

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
	p_led_opr->ctl(minor, status);
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 根据次设备号初始化LED */
	p_led_opr->init(minor);
	
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* 2. 定义自己的file_operations结构体                                              */
static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};

/* 4. 把file_operations结构体告诉内核：注册驱动程序                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init led_init(void)
{
	int err;
	int i;
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    //参数为0表示让内核自动为驱动程序分配一个未被占用的主设备号。
	major = register_chrdev(0, "100ask_led", &led_drv);  /* /dev/led */

    //创建了一个名为"100ask_led_class"的新设备类，并返回一个指向该设备类的指针。
	led_class = class_create(THIS_MODULE, "100ask_led_class");

    //PTR_ERR(led_class)将led_class指针中存储的错误代码提取出来，并作为函数的返回值。
    //通常情况下，如果一个指针指向的是一个错误代码，那么这个指针的值会被设置为一个负数。
    //因此，如果led_class指针的值是一个负数，那么PTR_ERR(led_class)将返回这个负数；否则返回0，表示没有错误发生。
	err = PTR_ERR(led_class);

    //在这个例子中，IS_ERR(led_class)将判断led_class指针是否指向一个错误代码。
    //如果class_create()函数创建设备类失败，那么led_class指针的值将被设置为一个错误代码，此时IS_ERR(led_class)将返回true，表示指针指向了一个错误代码。
    //如果class_create()函数创建设备类成功，那么led_class指针将指向一个设备类结构体，此时IS_ERR(led_class)将返回false，表示指针指向了一个有效的设备类结构体。
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "100ask_led");
		return -1;
	}

	for (i = 0; i < LED_NUM; i++)
        //在这个例子中，device_create()函数创建了一个名为"100ask_ledX"（X为设备编号）的设备节点，
        //并将该节点添加到系统中。其中，led_class是之前创建的设备类的指针，NULL表示没有父设备，
        //MKDEV(major, i)创建了设备号，NULL表示没有设备属性，"100ask_led%d"是设备节点的名称，其中%d将被替换为设备编号i。
		device_create(led_class, NULL, MKDEV(major, i), NULL, "100ask_led%d", i); /* /dev/100ask_led0,1,... */

	p_led_opr = get_board_led_opr();
	
	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{
	int i;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	for (i = 0; i < LED_NUM; i++)
		device_destroy(led_class, MKDEV(major, i)); /* /dev/100ask_led0,1,... */

	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "100ask_led");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


