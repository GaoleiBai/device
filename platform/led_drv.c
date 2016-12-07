#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/slab.h>
#include<asm/io.h>
#include<linux/platform_device.h>
#include"led.h"

static struct led_device *fs210_led_dev;

static int led_open(struct inode *inode,struct file *file)
{
	//unsigned long val=0;
	
	#if 0
	/*初始化硬件*/
	/*将GPC0[3],GPC0[4]设置为输出*/
	val =readl(fs210_led_dev->led_base+FS210_GPC0C0N);  //读gpc0con寄存器
	val &=~((0xf<<12)|(0xf<<16));
	val |=(0x1<<12)|(0x1<<16);
	writel(val,fs210_led_dev->led_base+FS210_GPC0C0N);
	#endif
	
	return 0;
}

static int led_close(struct inode *inode,struct file *file)
{
	//iounmap(gpc0con);
	
	return 0;
}

static ssize_t led_read(struct file *file, char __user *buf, size_t size, loff_t *opps)
{	
	return 0;
}

//write(fd,buf,4)
static ssize_t led_write(struct file *file, const char __user *buf, size_t size, loff_t *opps)
{
	int ret=0;
	unsigned int val=0;
	/*获取用户空间的数据
     *param1:目的
     *param2:源
     *param3:大小
     *返回值:失败返回>0,返回剩余的没有拷贝成功的字节数
     *       成功返回0
	 */
	ret =copy_from_user(&val,buf,size);
	if(ret)
		printk(KERN_ERR "copy data from user failed!\n");

	if(val){
		/*点灯*/
		writel(readl(fs210_led_dev->led_base+S5PV210_GPC0DAT)|(0x1<<fs210_led_dev->pin),fs210_led_dev->led_base+S5PV210_GPC0DAT);
	}else{
		/*灭灯*/
		writel(readl(fs210_led_dev->led_base+S5PV210_GPC0DAT)&~((0x1<<fs210_led_dev->pin),fs210_led_dev->led_base+S5PV210_GPC0DAT);
	}
	return ret?-EINVAL:size;
}

struct file_operations led_fops ={
	.owner	=THIS_MODULE,
	.open	=led_open,
	.release=led_close,
	.write 	=led_write,
	.read	=led_read,
};


/*匹配成功的时候,会调用probe函数*/
static int fs210_led_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct fs210_led_platdata *pdata;
	
	fs210_led_dev =kmalloc(sizeof(struct led_device),GFP_KERNEL);
	if(fs210_led_dev==NULL){
		dev_err(&pdev->dev,"no memory for malloc!\n");
		return -ENOMEM;
	}

	/*获取平台数据资源*/
	pdata =pdev->dev.platform_data;
	fs210_led_dev->pin =pdata->pin;
	
	/*1. 获取资源*/
	res =platform_get_resource(pdev,IORESOURCE_MEM,0);
	
	/*2. 映射,如果是中断资源,赶紧注册中断*/
	fs210_led_dev->led_base =ioremap(res->start, res->end-res->start+1);
	if(fs210_led_dev->led_base==NULL){
		dev_err(&pdev->dev,"no memory for ioremap!\n");
		ret= -ENOMEM;
		goto err0;
	}

	/*3. 初始化硬件:将GPC0_3,GPC0_4设置为输出*/
	writel((readl(fs210_led_dev->led_base+S5PV210_GPC0CON)&~(0xf<<4*fs210_led_dev->pin)|(0x1<<4*fs210_led_dev->pin),\
			fs210_led_dev->led_base+S5PV210_GPC0CON);
	
	/*4. 给用户提供接口*/
	/*4.1 申请设备号,注册字符设备,注册操作方法*/
	ret=register_chrdev(LED_MAJOR,"led_module",&led_fops);
	if(ret<0){
		dev_err(&pdev->dev,"register major failed!\n");
		ret= -EINVAL;
		goto err1;
	}
	/*4.2 创建设备文件*/
	/*3.创建设备文件*/
	fs210_led_dev->led_class =class_create(THIS_MODULE, "led_class");
	if(IS_ERR(fs210_led_dev->led_class)){
		printk(KERN_ERR "failed to create class!\n");
		ret =PTR_ERR(fs210_led_dev->led_class);
		goto err2;
	}

	fs210_led_dev->led_device =device_create(fs210_led_dev->led_class,NULL,MKDEV(LED_MAJOR,0),NULL,"led");
	if(IS_ERR(fs210_led_dev->led_device)){
		printk(KERN_ERR "failed to create class!\n");
		ret =PTR_ERR(fs210_led_dev->led_device);
		goto err3;
	}
	
	return 0;
	
err3:
	class_destroy(fs210_led_dev->led_class);
err2:
	unregister_chrdev(LED_MAJOR,"led_module");
err1:
	iounmap(fs210_led_dev->led_base);
err0:
	kfree(fs210_led_dev);
	return ret;
}


static int fs210_led_remove(struct platform_device *pdev)
{
	unregister_chrdev(LED_MAJOR,"led_module");
	iounmap(fs210_led_dev->led_base);
	device_destroy(fs210_led_dev->led_class,MKDEV(LED_MAJOR,0));
	class_destroy(fs210_led_dev->led_class);
	kfree(fs210_led_dev);
	return 0;
}

struct platform_device_id led_ids[]={
	[0]={
		.name			="mini210-led",
		.driver_data    =0,
	},
	[1]={
		.name			="smart210-led",
		.driver_data	=0,
	},
	[2]={
		.name			="fs210-led",
		.driver_data	=0,
	},
};

/*1.构建一个平台驱动结构体*/
struct platform_driver fs210_led_driver={
	.probe	=fs210_led_probe,
	.remove	=fs210_led_remove,
	.driver	={
		.owner  =THIS_MODULE,
		.name	="fs210-led",
	},
	.id_table	=led_ids,
};

static __init int led_drv_init(void)
{
	/*2.注册平台驱动
     *a.将平台驱动加入驱动链表
     *b.搜索平台总线的设备链表,没搜索一个设备,都会调用总线里面的match函数
     *  实现按名字匹配,匹配成功则调用驱动中的probe函数
	 */
	platform_driver_register(&fs210_led_driver);
	return 0;
}

static __exit void led_drv_exit(void)
{
	platform_driver_unregister(&fs210_led_driver);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");


