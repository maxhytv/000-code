//指定引脚


#include "led_resource.h"


//这种写法的主要目的是将LED资源的定义和获取分离，使得可以方便地在不同的函数和模块中使用同一个LED资源。
//具体地说，静态结构体变量的定义可以确保该变量只在当前文件中可见，而不会被其他文件访问，
//从而保证了对该变量的访问是安全的。
//同时，通过定义一个获取LED资源的函数，可以使得获取LED资源的过程更加灵活，
//可以在不同的函数和模块中使用同一个LED资源。
static struct led_resource board_A_led = {
	.pin = GROUP_PIN(3,1),
};

struct led_resource *get_led_resouce(void)
{
	return &board_A_led;
}


