#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "Delay_IO.h"


int main(int argc , char *argv[])
{
	char key;
        printf("Please select what you wnat to set:\n");
        printf("0) Set time of Delay OFF to 0 sec\n");
	printf("1) Set time of Delay OFF to 20 sec\n");
	printf("2) Set time of Delay OFF to 1 min\n"); 
	printf("3) Set time of Delay OFF to 5 min\n"); 
	printf("4) Set time of Delay OFF to 10 min\n"); 
	printf("5) Set time of Delay OFF to 30 min\n");
	printf("6) Set time of Delay OFF to 1 hour\n"); 
	printf("7) Set time of Delay OFF to 6 hour\n");
	printf("8) Set time of Delay OFF to 18 hour\n");
	printf("Please select the value of Delay OFF (0~8):");
        scanf("%c",&key);
        switch (key) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			DelayOFF_Set(key);
		break;
		default:
                        printf("The number of you select is wrong, please execute this program again.\n");
	}
	printf("The setting vlaue will be available after you restar system.\n");
	exit(0);
}

