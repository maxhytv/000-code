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
	//写的时候考虑，成功还是失败，err、
	//写什么？status
	int err;
	char status;
	
	//inode 结构体，用于存储文件的元数据信息，例如文件类型、权限、大小、创建时间等。
	//在内核中，可以通过文件描述符 file 来获取文件对应的 inode 结构体指针，以便进行对文件的操作。
	//获取了文件的 inode 结构体指针之后，就可以对文件进行操作，例如读取文件内容、修改文件权限等。
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);

	/* 根据次设备号和status控制LED */
	p_led_opr->ctl(minor, status);
	//返回写入的字节数，本函数中固定为1字节。
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	//次设备号用于标识同一驱动程序中的不同设备实例
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

	//字符设备驱动的注册代码
	//major 参数是字符设备的主设备号，如果传入 0，则表示让内核自动分配主设备号
	major = register_chrdev(0, "100ask_led", &led_drv);  /* /dev/led */

	//class_create() 函数返回的是一个指向创建的字符设备类的指针。
	//如果函数执行成功，则返回一个有效的指针，否则返回一个空指针（NULL）。
	led_class = class_create(THIS_MODULE, "100ask_led_class");

	//err = PTR_ERR(led_class) 的作用是获取 class_create() 函数返回的实际错误码（如果有），
	// 以便进行错误处理。如果没有错误，则 err 变量的值将是 0，不会影响后续代码的执行。
	err = PTR_ERR(led_class);、

	//IS_ERR() 是一个宏定义，用于判断一个指针是否为错误指针。
	//在 Linux 内核中，当某些函数返回一个指针时，如果发生错误，
	//该函数会返回一个错误指针，而不是有效的指针。
	//这种错误指针通常是一个负数，表示错误代码。
	//为了方便判断指针是否为错误指针，内核提供了 IS_ERR() 宏定义。
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "100ask_led");
		return -1;
	}

	
	//led_class 参数是一个指向字符设备类的指针，表示将要创建的字符设备属于 led_class 类。
	//NULL 参数表示该设备没有父设备。
	//MKDEV(major, i) 函数用于将主设备号 major 和次设备号 i 组合成一个设备号，并将其赋值给 devt 参数。
	//NULL 参数表示该设备没有驱动程序私有数据。
	//"100ask_led%d" 是设备文件名的格式字符串，其中 %d 表示次设备号，将在后续代码中被替换为实际的次设备号。
	for (i = 0; i < LED_NUM; i++)
		device_create(led_class, NULL, MKDEV(major, i), NULL, "100ask_led%d", i); /* /dev/100ask_led0,1,... */
	
	//用与接受static struct led_operations board_demo_led_opr
	p_led_opr = get_board_led_opr();
	
	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{
	int i;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//销毁设备节点
	for (i = 0; i < LED_NUM; i++)
		device_destroy(led_class, MKDEV(major, i)); /* /dev/100ask_led0,1,... */

	//销毁已经创建的类
	class_destroy(led_class);

	//注销已经注册的字符设备
	unregister_chrdev(major, "100ask_led");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


