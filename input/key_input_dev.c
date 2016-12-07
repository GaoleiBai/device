
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>

#include <plat/irqs.h>
#include <mach/irqs.h>
#include <mach/gpio.h>

#include "key_input.h"

struct key_desc fs_key_set[] = {
	[0] = {
		.name ="k1_up",
		.gpionum = S5PV210_GPH0(0),
		.event = {
			.type = EV_KEY,
			.code = KEY_UP,
			.value = 0,
		},
	},
	[1] = {
		.name ="k2_down",
		.gpionum = S5PV210_GPH0(1),
		.event = {
			.type = EV_KEY,
			.code = KEY_DOWN,
			.value = 0,
		},
	},
	[2] = {
		.name ="k3_left",
		.gpionum = S5PV210_GPH0(2),
		.event = {
			.type = EV_KEY,
			.code = KEY_LEFT,
			.value = 0,
		},
	},
	[3] = {
		.name ="k4_right",
		.gpionum = S5PV210_GPH0(3),
		.event = {
			.type = EV_KEY,
			.code = KEY_RIGHT,
			.value = 0,
		},
	},
	[4] = {
		.name ="k5_enter",
		.gpionum = S5PV210_GPH0(4),
		.event = {
			.type = EV_KEY,
			.code = KEY_ENTER,
			.value = 0,
		},
	},
	[5] = {
		.name ="k5_back",
		.gpionum = S5PV210_GPH0(5),
		.event = {
			.type = EV_KEY,
			.code = KEY_BACK,
			.value = 0,
		},
	},
	[6] = {
		.name ="k7_home",
		.gpionum = S5PV210_GPH2(6),
		.event = {
			.type = EV_KEY,
			.code = KEY_HOME,
			.value = 0,
		},
	},
	[7] = {
		.name ="k8_down",
		.gpionum = S5PV210_GPH2(7),
		.event = {
			.type = EV_KEY,
			.code = KEY_POWER,
			.value = 0,
		},
	},
	

};
void  fs_key_plat_release(struct device *dev)
{
}

struct key_platdata fs_key_platdata ={
	.key_nr = ARRAY_SIZE(fs_key_set),
	.key_set = fs_key_set,
};

static struct platform_device key_pdev = {
		.name = "fs210_key",
		.id = -1,
		.dev = {
			.release = fs_key_plat_release,
			.platform_data = fs_key_platdata,
		}
};

void __init s3c_key_set_platdata(struct key_platdata *pd)
{
	struct key_platdata *npd;

	if (!pd) {
		printk(KERN_ERR "%s: no platform data\n", __func__);
		return;
	}

	npd = kmemdup(pd, sizeof(struct key_platdata), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);

	key_pdev.dev.platform_data = npd;
}

static int __init key_dev_init(void)
{
	printk("-------%s------------\n", __FUNCTION__);

	s3c_key_set_platdata(&fs_key_platdata);
	
	return platform_device_register(&key_pdev);
}

static void __exit key_dev_exit(void)
{
	printk("-------%s------------\n", __FUNCTION__);

	platform_device_unregister(&key_pdev);
}

module_init(key_dev_init);
module_exit(key_dev_exit);
MODULE_LICENSE("GPL");

