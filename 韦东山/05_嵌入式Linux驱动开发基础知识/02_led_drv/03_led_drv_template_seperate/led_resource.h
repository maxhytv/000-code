#ifndef _LED_RESOURCE_H
#define _LED_RESOURCE_H

/* GPIO3_0 */
/* bit[31:16] = group */
/* bit[15:0]  = which pin */
#define GROUP(x) (x>>16)
#define PIN(x)   (x&0xFFFF)

//g<<16 表示将组号 g 左移 16 位，
//然后使用按位或操作符 | 将左移后的组号和引脚号 p 进行按位或操作，
//得到一个 32 位的整数值，表示一个 GPIO 引脚。
#define GROUP_PIN(g,p) ((g<<16) | (p))

struct led_resource {
	int pin;
};

struct led_resource *get_led_resouce(void);

#endif

