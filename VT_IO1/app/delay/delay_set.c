#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#define BASEADDR1 0x70         // Base address
#define BASEADDR2 0x71         // Base address


int DelayON_Set(char setting)
{
	int bfr=0,bfr1=0,fh,cnt;	
  	fh = open("/dev/NEXCOM_IO", O_RDWR);
	if (fh==0)
	{
		printf("NEXCOM_IO driver cannot be opened.\n");
		return -1;
	}
	bfr=0x47;
	cnt = write(fh,&bfr,BASEADDR1);
        read(fh,&bfr1,BASEADDR2);

	switch (setting) {
		case '0':
			bfr=bfr1 & 0x47;
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '1':
			bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0x80; 
			cnt = write(fh,&bfr,BASEADDR2);
		break;
		case '2':
                	bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0x88; 
			cnt = write(fh,&bfr,BASEADDR2);
       		break;
	 	case '3':
               		bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0x90; 
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '4':
                	bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0x98; 
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '5':
                        bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0xa0; 
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '6':
                        bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0xa9; 
			cnt = write(fh,&bfr,BASEADDR2);
                 break;
		 case '7':
                        bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0xb0; 
			cnt = write(fh,&bfr,BASEADDR2);
 	       	 break;
		 case '8':
                        bfr1=bfr1 & 0x47;	
			bfr =bfr1 | 0xb8; 
			cnt = write(fh,&bfr,BASEADDR2);
                 break;
		 default:
			printf("The number of you select is wrong, please execute this program again.\n");
			close(fh);
			return -1;


        }  
//	read(fh,&bfr,BASEADDR2);		 	
// 	printf("0x47=%x\n",bfr);
  	close(fh);
	return 0;
}

int DelayOFF_Set(char setting)
{
	int bfr=0,bfr1=0,fh,cnt;	
  	fh = open("/dev/NEXCOM_IO", O_RDWR);
	if (fh==0)
	{
		printf("NEXCOM_IO driver cannot be opened.\n");
		return -1;
	}
	bfr=0x47;
	cnt = write(fh,&bfr,BASEADDR1);
        read(fh,&bfr1,BASEADDR2);


	switch (setting) {
		case '0':
			bfr=bfr1 & 0xb8;
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '1':
			bfr1=bfr1 & 0xb8;
			bfr =bfr1 | 0x40; 
			cnt = write(fh,&bfr,BASEADDR2);
		break;
		case '2':
                	bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x41; 
			cnt = write(fh,&bfr,BASEADDR2);
       		break;
	 	case '3':
               		bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x42; 
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '4':
                	bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x43; 
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '5':
                        bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x44; 
			cnt = write(fh,&bfr,BASEADDR2);
                break;
		case '6':
                        bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x45; 
			cnt = write(fh,&bfr,BASEADDR2);
                 break;
		 case '7':
                        bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x46; 
			cnt = write(fh,&bfr,BASEADDR2);
 	       	 break;
		 case '8':
                        bfr1=bfr1 & 0xb8;	
			bfr =bfr1 | 0x47; 
			cnt = write(fh,&bfr,BASEADDR2);
                 break;
		 default:
			printf("The number of you select is wrong, please execute this program again.\n");
			close(fh);
			return -1;

        }  
	//read(fh,&bfr,BASEADDR2);		 	
 	//printf("0x47=%x\n",bfr);
	
	close(fh);
	return 0;
}





