#ifndef __ASM_LED_H
#define __ASM_LED_H

#define S5PV210_PA_LED	0xE0200060
#define LED_MAJOR 		250
#define S5PV210_GPC0CON	0x0
#define S5PV210_GPC0DAT	0x04

struct led_device{
	struct class *led_class;
	struct device *led_device;
	void __iomem  *led_base;
	int			  pin;
};

/*构建平台数据结构体*/
struct fs210_led_platdata{
	int pin;
};

#endif
