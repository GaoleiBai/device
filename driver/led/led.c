1   #include .../*包含内核中的多个头文件*/ 
2    
3   /*设备结构体*/ 
4   struct light_dev 
5   { 
6     struct cdev cdev; /*字符设备 cdev 结构体*/ 
7     unsigned char value; /*LED 亮时为 1，熄灭时为 0，用户可读写此值*/ 
8   }; 
9    
10  struct light_dev *light_devp; 
11  int light_major = LIGHT_MAJOR; 
12   
13  MODULE_AUTHOR("Song Baohua"); 
14  MODULE_LICENSE("Dual BSD/GPL"); 
15   
16  /*打开和关闭函数*/
17  int light_open(struct inode *inode, struct file *filp) 
18  { 
19    struct light_dev *dev; 
20    /* 获得设备结构体指针 */ 
21    dev = container_of(inode->i_cdev, struct light_dev, cdev); 
22    /* 让设备结构体作为设备的私有信息 */ 
23    filp->private_data = dev; 
24    return 0; 
25  } 
26  int light_release(struct inode *inode, struct file *filp) 
27  { 
28    return 0; 
29  } 
30   
31  /*写设备:可以不需要 */ 
32  ssize_t light_read(struct file *filp, char _ _user *buf, size_t count, 
33    loff_t*f_pos) 
34  { 
35    struct light_dev *dev = filp->private_data; /*获得设备结构体 */ 
36   
37    if (copy_to_user(buf, &(dev->value), 1)) 
38    { 
39      return  - EFAULT; 
40    } 
41    return 1; 
42  } 
43   
44  ssize_t light_write(struct file *filp, const char _ _user *buf, size_t count, 
45    loff_t *f_pos) 
46  { 
47    struct light_dev *dev = filp->private_data; 
48   
49    if (copy_from_user(&(dev->value), buf, 1)) 
50    { 
51      return  - EFAULT; 
52    } 
53    /*根据写入的值点亮和熄灭 LED*/ 
54    if (dev->value == 1)
55      light_on(); 
56    else 
57      light_off(); 
58   
59    return 1; 
60  } 
61   
62  /* ioctl 函数 */ 
63  int light_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, 
64    unsigned long arg) 
65  { 
66    struct light_dev *dev = filp->private_data; 
67   
68    switch (cmd) 
69    { 
70      case LIGHT_ON: 
71        dev->value = 1; 
72        light_on(); 
73        break; 
74      case LIGHT_OFF: 
75        dev->value = 0; 
76        light_off(); 
77        break; 
78      default: 
79        /* 不能支持的命令 */ 
80        return  - ENOTTY; 
81    } 
82   
83    return 0; 
84  } 
85   
86  struct file_operations light_fops = 
87  { 
88    .owner = THIS_MODULE, 
89    .read = light_read, 
90    .write = light_write, 
91    .ioctl = light_ioctl, 
92    .open = light_open, 
93    .release = light_release,
94  }; 
95   
96  /*设置字符设备 cdev 结构体*/ 
97  static void light_setup_cdev(struct light_dev *dev, int index) 
98  { 
99    int err, devno = MKDEV(light_major, index); 
100  
101   cdev_init(&dev->cdev, &light_fops); 
102   dev->cdev.owner = THIS_MODULE; 
103   dev->cdev.ops = &light_fops; 
104   err = cdev_add(&dev->cdev, devno, 1); 
105   if (err) 
106     printk(KERN_NOTICE "Error %d adding LED%d", err, index); 
107 } 
108  
109 /*模块加载函数*/ 
110 int light_init(void) 
111 { 
112   int result; 
113   dev_t dev = MKDEV(light_major, 0); 
114  
115   /* 申请字符设备号*/ 
116   if (light_major) 
117     result = register_chrdev_region(dev, 1, "LED"); 
118   else 
119   { 
120     result = alloc_chrdev_region(&dev, 0, 1, "LED"); 
121     light_major = MAJOR(dev); 
122   } 
123   if (result < 0) 
124     return result; 
125  
126   /* 分配设备结构体的内存 */ 
127   light_devp = kmalloc(sizeof(struct light_dev), GFP_KERNEL); 
128   if (!light_devp)   /*分配失败*/ 
129   { 
130     result =  - ENOMEM; 
131     goto fail_malloc; 
132   } 
133   memset(light_devp, 0, sizeof(struct light_dev));
134   light_setup_cdev(light_devp, 0); 
135   light_init(); 
136   return 0; 
137    
138   fail_malloc: unregister_chrdev_region(dev, light_devp); 
139   return result; 
140 } 
141  
142 /*模块卸载函数*/ 
143 void light_cleanup(void) 
144 { 
145   cdev_del(&light_devp->cdev); /*删除字符设备结构体*/ 
146   kfree(light_devp); /*释放在 light_init 中分配的内存*/ 
147   unregister_chrdev_region(MKDEV(light_major, 0), 1); /*删除字符设备*/ 
148 } 
149  
150 module_init(light_init); 
151 module_exit(light_cleanup);
