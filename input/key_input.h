#ifndef __KEY_INPUT_H__
#define __KEY_INPUT_H__

#include <linux/input.h>

// 按键信息
struct key_desc{
	char *name;
	int gpionum;
	struct input_event event;
	struct input_dev  *i_dev;
};

//key的平台设备私有数据
struct key_platdata{
	int key_nr;
	struct key_desc *key_set;
};

//key的平台驱动私有数据
struct key_input_dev{
	struct input_dev *i_dev;
	struct key_platdata *pd;
	struct device *dev;
};


#endif
