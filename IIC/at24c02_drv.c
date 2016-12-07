#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/uaccess.h>
//#include <linux/err.h>

struct e2prom_dev{
	int dev_major;
	struct i2c_client *client;
	struct class *cls;
	struct device *dev;
};

struct e2prom_dev *at24c02_dev;


static int i2c_read_bytes(char *buf, int count)
{
	int ret = 0;
	struct i2c_client *client = at24c02_dev->client;
	struct i2c_msg msg;


	msg.addr = client->addr;
	msg.flags |= I2C_M_RD,
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret < 0)
		printk(KERN_ERR"i2c_transfer error\n");
	return ret;
}

static int i2c_write_bytes(char *buf, int count)
{
	int ret = 0;
	struct i2c_client *client = at24c02_dev->client;
	struct i2c_msg msg;
	
	msg.addr = client->addr;
	msg.flags = 0,
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret < 0)
		printk(KERN_ERR"i2c_transfer error\n");
	return ret;
}


static int at24c02_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t at24c02_write(struct file *file, const char __user *buf, size_t size, loff_t *opps)
{
	int ret=0;
	char  *tmp;
	tmp =kmalloc(size,GFP_KERNEL);
	if(tmp==NULL)
	{
		printk("mallo failed!\n");
		return -ENOMEM;
	}

	ret =copy_from_user(tmp,buf,size);
	if(ret)
	{
		printk("copy data faile!\n");
		ret =-EFAULT;
		goto err_0;
	}
	
	ret=i2c_write_bytes(tmp,size);
	if(ret < 0){
		printk("write byte failed!\n");
		ret =-EINVAL;
		goto err_0;
	}
	
	kfree(tmp);
	return size;
	
err_0:
	kfree(tmp);
	return ret;
}

static ssize_t at24c02_read(struct file *file, char __user *buf, size_t size, loff_t *opps)
{
	int ret=0;
	char  *tmp;
	tmp =kmalloc(size,GFP_KERNEL);
	if(tmp==NULL)
	{
		printk("mallco failed!\n");
		return -ENOMEM;
	}
	ret = i2c_read_bytes(tmp, size);
	if(ret < 0)
	{
		printk("write byte failed!\n");
		ret =-EINVAL;
		goto err_0;
	}

	ret = copy_to_user(buf, tmp, size);
	if(ret)
	{
		printk("copy data faile!\n");
		ret =-EFAULT;
		goto err_0;
	}

	kfree(tmp);

	return ret ? -EINVAL : size;

err_0:
	kfree(tmp);
	return ret;
}

int at24c02_close(struct inode *inode, struct file *filp)
{
	return 0;
}



static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = at24c02_open,
	.read = at24c02_read,
	.write = at24c02_write,
	.release = at24c02_close,
};


int at24c02_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	printk("---------%s----------\n", __FUNCTION__);
	// 编写字符设备框架
	int ret;
	ret = i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA| I2C_FUNC_SMBUS_WORD_DATA);
	if(!ret)
	{
		printk("function is not support\n");
		return -ENODEV;
	}
	
	at24c02_dev = kzalloc(sizeof(struct e2prom_dev),  GFP_KERNEL);
	if(!at24c02_dev)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	
	at24c02_dev->client = client;

	at24c02_dev->dev_major = register_chrdev(0, "at24c02", &fops);
	if(at24c02_dev->dev_major < 0)
	{
		printk(KERN_ERR "register_chrdev error\n");
		ret =  -ENODEV;
		goto err_0;
		
	}

	at24c02_dev->cls = class_create(THIS_MODULE, "e2prom");
	if(IS_ERR(at24c02_dev->cls))
	{
		printk(KERN_ERR "class_create error\n");
		ret =  PTR_ERR(at24c02_dev->cls);
		goto err_1;
		
	}

	at24c02_dev->dev = device_create(at24c02_dev->cls, NULL, MKDEV(at24c02_dev->dev_major,0),
					NULL, "i2c-e2prom0");
	if(IS_ERR(at24c02_dev->dev))
	{
		printk(KERN_ERR "class_create error\n");
		ret =  PTR_ERR(at24c02_dev->dev);
		goto err_2;
		
	}

	return 0;

err_3:
	device_destroy(at24c02_dev->cls, MKDEV(at24c02_dev->dev_major, 0));
err_2:
	class_destroy(at24c02_dev->cls);
err_1:
	unregister_chrdev(at24c02_dev->dev_major, "at24c02");
err_0:
	kfree(at24c02_dev);
	return ret;

}


int at24c02_remove(struct i2c_client *client)
{
	printk("---------%s----------\n", __FUNCTION__);
	device_destroy(at24c02_dev->cls, MKDEV(at24c02_dev->dev_major, 0));
	class_destroy(at24c02_dev->cls);
	unregister_chrdev(at24c02_dev->dev_major, "at24c02");
	kfree(at24c02_dev);
	return 0;
}

//i2c总线比较特殊，一定要有一个id表
struct i2c_device_id e2prom_table[]={
	[0]={
		.name			="24c02",
		.driver_data 	=0,
	},
	[1]={
		.name			="24c08",
		.driver_data 	=0,
	},
};


static struct i2c_driver at24c02_drv = {
	.probe = at24c02_probe,
	.remove = at24c02_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "at24c02",
	},
	.id_table = e2prom_table,
};

static int __init at24c02_init(void)
{
	printk("---------%s----------\n", __FUNCTION__);
	return i2c_add_driver(&at24c02_drv);
}

static void __exit at24c02_exit(void)
{
	printk("---------%s----------\n", __FUNCTION__);
	i2c_del_driver(&at24c02_drv);
}

module_init(at24c02_init)
module_exit(at24c02_exit);
MODULE_LICENSE("GPL");

