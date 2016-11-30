nt main(void)
{
  int fd,res,i;
  struct sigaction saio; /*definition of signal axtion */
  char buf[255];
  fd_set rd;
  fd = 0;
  /*打开串口*/
  if((fd = open_port(fd,1))<0)
  {
    perror("open_port error!\n");
    return (-1);
  }
  /* install the signal handle before making the device asynchronous*/
  saio.sa_handler = signal_handler_IO;
  sigemptyset(&saio.sa_mask);
  //saio.sa_mask = 0; 必须用sigemptyset函数初始话act结构的sa_mask成员 

  saio.sa_flags = 0;
  saio.sa_restorer = NULL;
  sigaction(SIGIO,&saio,NULL);

  /* allow the process to recevie SIGIO*/
  fcntl(fd,F_SETOWN,getpid());
  /* Make the file descriptor asynchronous*/
  fcntl(fd,F_SETFL,FASYNC);
  
  /*设置串口*/
  if((i= set_opt(fd,115200,8,'N',1))<0)
  {
    perror("set_opt error!\n");
    return (-1);
  }
  /* loop while waiting for input,normally we would do something useful here*/
  while(STOP == FALSE)
  {
    usleep(100000);
    /* after receving SIGIO ,wait_flag = FALSE,input is availabe and can be read*/
    if(wait_flag == FALSE)
    {
          memset(buf,0,255);
      res = read(fd,buf,255);
       printf("nread=%d,%s\n",res,buf);
      if(res == 1)
        STOP = TRUE; /*stop loop if only a CR was input */
       wait_flag = TRUE; /*wait for new input*/
    }
  }
  close(fd);
  return 0; 
}
/******************************************
 信号处理函数，设备wait_flag=FASLE
 ******************************************************/
void signal_handler_IO(int status)
{
   printf("received SIGIO signale.\n");
  wait_flag = FALSE; 
}
