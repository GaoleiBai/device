#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*./led_test on 点灯
 *./led_test off 灭灯
 */
int main(int argc,char **argv)
{
	int fd;
	int buf=0;
	
	/*1.打开驱动/打开设备文件*/
	fd=open("/dev/led",O_RDWR);
	if(fd<0){
		perror("open failed");
		exit(1);
	}	

	if(strcmp(argv[1],"on")==0){
		buf =1;
	}else{
		buf =0;
	}
	
	/*2.写数据*/
	if(write(fd,&buf,4)!=4){
		perror("write failed");
		exit(1);
	}
	
	close(fd);
	return 0;
}

