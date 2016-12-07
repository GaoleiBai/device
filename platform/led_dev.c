#include<linux/init.h>
#include<linux/module.h>
#include<linux/ioport.h>
#include<linux/platform_device.h>
#include"led.h"

/*平台数据资源:板级相关的资源*/
struct fs210_led_platdata cfg_platdata={
	.pin 	=3;
};


/*1.构建资源结构体:CPU架构相关的资源*/
struct resource led_resource[]={
	[0]={
		.start  =S5PV210_PA_LED,
		.end	=S5PV210_PA_LED+SZ_8-1,
		.flags	=IORESOURCE_MEM,
	},
};

static void led_release(struct device *dev)
{
	
}

/*2.构建平台设备结构体*/
struct platform_device s3c_led_device={
	.name			="fs210-led",
	.id				=-1,
	.resource		=led_resource,
	.num_resources	=ARRAY_SIZE(led_resource),
	.dev			={
		.release		=led_release,
		.platform_data  =&cfg_platdata;
	},
};

static __init int led_dev_init(void)
{
	/*3.注册平台设备*/
	platform_device_register(&s3c_led_device);
	return 0;
}

static __exit void led_dev_exit(void)
{
	platform_device_unregister(&s3c_led_device);	
}
module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");


