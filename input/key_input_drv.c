#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#include <plat/irqs.h>
#include <mach/irqs.h>
#include <mach/gpio.h>

#include "key_input.h"

irqreturn_t key_input_handle(int irq, void *dev_id)
{
	printk("-------%s------------\n", __FUNCTION__);
	struct key_desc tmp = *((struct key_desc *)dev_id);
	struct input_event event = tmp.event;
	
	printk("------%s :  code = %d------\n",__FUNCTION__, tmp.event.code);

	int val = gpio_get_value(tmp.gpionum);

	if(val)
	{
		event.value = 0;
		input_event(tmp.i_dev, event.type, event.code, event.value);
		input_sync(tmp.i_dev);
	}else
	{
		event.value = 1;
		input_event(tmp.i_dev, event.type, event.code, event.value);
		input_sync(tmp.i_dev);
	}

	
	return IRQ_HANDLED;
}


static int input_key_probe(struct platform_device *pdev)
{
	printk("-------%s------------\n", __FUNCTION__);
	int i;
	int ret ;
	int num;
	int irqno;
	struct key_input_dev *keydev;
	
	struct key_platdata *pdata = pdev->dev.platform_data;
	struct device *dev = &pdev->dev;

	//分配空间并初始化驱动中私有数据
	keydev = kzalloc(sizeof(struct key_input_dev), GFP_KERNEL);
	if(keydev == NULL)
	{
		printk(KERN_ERR "malloc error\n");
		return -ENOMEM;
	}

	keydev->dev = dev;
	keydev->pd = pdata;

	// 分配input设备
	keydev->i_dev =  input_allocate_device();
	if(keydev->i_dev == NULL)
	{
		printk(KERN_ERR "input_allocate_device error\n");
		ret = -ENOMEM;
		goto err_0;
	}

	keydev->i_dev->name = pdev->name;
	keydev->i_dev->phys = "fs210_key/input0";

	keydev->i_dev->id.bustype = BUS_HOST;
	keydev->i_dev->id.vendor =0x4598;
	keydev->i_dev->id.product = 0x1;
	keydev->i_dev->id.version = 0x11;


	// BIT_MASK表示将某个值左移到第几位，如果超过32bit话就需要除模32
	//evbit每个元素都是32bit，而元素的个数是可变的
	//以下的意思是使得input设备支持key和syn同步事件
	keydev->i_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_SYN);
	
	//申请中断
	for(i=0; i<pdata->key_nr; i++)
	{
		struct key_desc *desc = &pdata->key_set[i];
		desc->i_dev = keydev->i_dev;
		
		irqno = gpio_to_irq(desc->gpionum);
		ret = request_irq(irqno, key_input_handle, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, 
					desc->name, desc);
		
		num = i;
		if(ret != 0)
		{
			printk(KERN_ERR "request_irq for %s error\n", desc->name);
			goto err_2;
			
		}
		set_bit(desc->event.code, keydev->i_dev->keybit);
	}

	platform_set_drvdata(pdev, keydev);
	
	//注册input设备
	ret = input_register_device(keydev->i_dev);
	if (ret) {
		printk( KERN_ERR "unable to register input device\n");
		goto err_2;
	}
	
	return 0;

err_2:
	for(i=num--; i>0; i--)
	{
		struct key_desc *desc = &pdata->key_set[i];
		irqno = gpio_to_irq(desc->gpionum);
		free_irq(irqno, desc);
	}	
err_1:
	input_free_device(keydev->i_dev);
err_0:
	kfree(keydev);

	return ret;
}

static int input_key_remove(struct platform_device *pdev)
{
	printk("-------%s------------\n", __FUNCTION__);
	int irqno, i;
	struct key_input_dev *keydev = platform_get_drvdata(pdev);
	struct key_platdata *pdata = pdev->dev.platform_data;
	
	for(i=0; i<pdata->key_nr; i++)
	{
		struct key_desc *desc = &pdata->key_set[i];
		irqno = gpio_to_irq(desc->gpionum);
		free_irq(irqno, desc);
	}	
	
	input_unregister_device( keydev->i_dev);
	input_free_device(keydev->i_dev);
	kfree(keydev);
	return 0;
}



static struct platform_driver key_pdrv = {
	.probe = input_key_probe,
	.remove = input_key_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "fs210_key",
	},
	// 做成ko时最好不要有id
};


static int __init key_drv_init(void)
{
	printk("-------%s------------\n", __FUNCTION__);
	return platform_driver_register(&key_pdrv);
}

static void __exit key_drv_exit(void)
{
	printk("-------%s------------\n", __FUNCTION__);

	platform_driver_unregister(&key_pdrv);
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");


