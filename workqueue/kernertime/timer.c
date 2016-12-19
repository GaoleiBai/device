#include <linux/module.h>  
#include <linux/init.h>  
  
#include <linux/sched.h>  
#include <linux/timer.h>  
  
#if 0  //定义并初始化定时器结构体timer_list。  
/*include/linux/timer.h*/  
struct timer_list {  
    struct list_head entry;  
    unsigned long expires; //设置在执行定时器处理函数的时间  
  
    void (*function)(unsigned long); //定时器处理函数  
    unsigned long data; //处理函数的传参  
  
    struct tvec_base *base;  
      
    #ifdef CONFIG_TIMER_STATS  
        void *start_site;  
        char start_comm[16];  
        int start_pid;  
    #endif  
      
};  
#endif  
  
struct timer_list my_timer; //1.定义定时器结构体timer_list  
  
void timer_func(unsigned long data) //2.定义定时器处理函数  
{  
    printk("time out![%d] [%s]\n", (int)data, current->comm); //打印当前进程  
}  
  
static int __init test_init(void) //模块初始化函数  
{  
     init_timer(&my_timer); //1.初始化timer_list结构  
  
     my_timer.expires = jiffies + 5*HZ; //2.设定定时器处理函数触发时间为5秒  
     my_timer.function = timer_func; //2.给结构体指定定时器处理函数  
     my_timer.data = (unsigned long)99; //2.设定定时器处理函数的传参  
  
     add_timer(&my_timer); //3.激活定时器  
     printk("hello timer,current->comm[%s]\n", current->comm);  
     return 0;  
 }  
  
static void __exit test_exit(void) //模块卸载函数  
{  
    printk("good bye timer\n");  
}  

