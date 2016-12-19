#include <linux/module.h>  
#include <linux/init.h>  
  
#include <linux/interrupt.h>  
#include <linux/workqueue.h>  
  
#define DEBUG_SWITCH 1  
#if DEBUG_SWITCH  
    #define P_DEBUG(fmt, args...) printk("<1>" "<kernel>[%s]"fmt, __FUNCTI ON__, ##args)  
#else  
    #define P_DEBUG(fmt, args...) printk("<7>" "<kernel>[%s]"fmt, __FUNCTI ON__, ##args)  
#endif  
  
 struct workqueue_struct *ZP1015_wq; //1.定义工作队列  
 struct work_struct ZP1015_work;    //2定义work结构体  
  
 void ZP1015_func(struct work_struct *work) //2实现处理函数  
 {  
    printk("hello ZP1015!\n");  
 }  
  
irqreturn_t irq_handler(int irqno, void *dev_id)  
 {  
     printk("key down\n");  
     queue_work(ZP1015_wq ,&ZP1015_work); //3调度任务  
     return IRQ_HANDLED;  
 }  
 static int __init test_init(void) //模块初始化函数  
 {  
     int ret;  
  
     /*work*/  
     ZP1015_wq = create_workqueue("ZP1015"); //1初始化工作对列  
     INIT_WORK(&ZP1015_work, ZP1015_func);  //2初始化work结构体  
  
     ret = request_irq(IRQ_EINT1, irq_handler,  
     IRQF_TRIGGER_FALLING, "key INT_EINT1", NULL);  
       
     if(ret){  
        P_DEBUG("request irq failed!\n");  
        return ret;  
    }  
  
     printk("hello irq\n");  
     return 0;  
 }  
  
 static void __exit test_exit(void) //模块卸载函数  
 {  
     flush_workqueue(ZP1015_wq);    //4刷新工作队列  
     destroy_workqueue(ZP1015_wq); //4注销工作队列  
     free_irq(IRQ_EINT1, NULL);  
     printk("good bye irq\n");  
 }  
  
 module_init(test_init);  
 module_exit(test_exit);  
  
 MODULE_LICENSE("GPL");  
 MODULE_AUTHOR("ZP1015");  
 MODULE_VERSION("v0.1");  
