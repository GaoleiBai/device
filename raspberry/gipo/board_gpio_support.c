#include <linux/miscdevice.h>  
#include <linux/delay.h>  
#include <asm/irq.h>  
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/init.h>  
#include <linux/mm.h>  
#include <linux/fs.h>  
#include <linux/types.h>  
#include <linux/delay.h>  
#include <linux/moduleparam.h>  
#include <linux/slab.h>  
#include <linux/errno.h>  
#include <linux/ioctl.h>  
#include <linux/cdev.h>  
#include <linux/string.h>  
#include <linux/list.h>  
#include <linux/pci.h>  
#include <asm/uaccess.h>  
#include <asm/atomic.h>  
#include <asm/unistd.h>  
#include <asm/io.h>  
#include <asm/uaccess.h>  
#include <linux/ioport.h>  
  
#include "bcm2835.h"  
  
int bcm2835_gpio_fsel(uint8_t pin, uint8_t mode)  
{  
    //初始化GPIOB功能选择寄存器的物理地址  
    volatile uint32_t * bcm2835_gpio = (volatile uint32_t *)ioremap(BCM2835_GPIO_BASE, 16);  
    volatile uint32_t * bcm2835_gpio_fsel = bcm2835_gpio + BCM2835_GPFSEL0/4 + (pin/10);  
    uint8_t   shift = (pin % 10) * 3;  
    uint32_t  value = mode << shift;  
    *bcm2835_gpio_fsel = *bcm2835_gpio_fsel | value;  
  
    printk("fsel address: 0x%lx : %x\n", bcm2835_gpio_fsel, *bcm2835_gpio_fsel);  
  
    return 0;  
}  
  
int bcm2835_gpio_set(uint8_t pin)  
{  
    //GPIO输出功能物理地址  
    volatile uint32_t * bcm2835_gpio = (volatile uint32_t *)ioremap(BCM2835_GPIO_BASE, 16);  
    volatile uint32_t * bcm2835_gpio_set = bcm2835_gpio + BCM2835_GPSET0/4 + pin/32;  
    uint8_t   shift = pin % 32;  
    uint32_t  value = 1 << shift;  
    *bcm2835_gpio_set = *bcm2835_gpio_set | value;  
  
    printk("set address:  0x%lx : %x\n", bcm2835_gpio_set, *bcm2835_gpio_set);  
  
    return 0;  
}  
  
int bcm2835_gpio_clr(uint8_t pin)  
{  
   //GPIO清除功能物理地址  
    volatile uint32_t * bcm2835_gpio = (volatile uint32_t *)ioremap(BCM2835_GPIO_BASE, 16);  
    volatile uint32_t * bcm2835_gpio_clr = bcm2835_gpio + BCM2835_GPCLR0/4 + pin/32;  
    uint8_t   shift = pin % 32;  
    uint32_t  value = 1 << shift;  
    *bcm2835_gpio_clr = *bcm2835_gpio_clr | value;  
      
    printk("clr address:  0x%lx : %x\n", bcm2835_gpio_clr, *bcm2835_gpio_clr);  
  
    return 0;  
}  
