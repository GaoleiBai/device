#include<linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<stdio.h>

struct input_event ev_button;

int  main(int argc,char **argv)
{
	int fd;
	int ret;
	fd =open(argv[1],O_RDONLY);
	if(fd<0)
	{
		printf("cannot open /dev/event0!\n");
		return -1;
	}
	
	while(1)
	{
		read(fd,&ev_button,sizeof(struct input_event));
		//printf("code=%d\n",ev_button.code);
		if(ev_button.type!=EV_KEY)
			continue;
		switch(ev_button.code)
		{
			case KEY_UP:
				if(ev_button.value)
				{
					printf("key1 is pressed!\n");
				}
				else
				{
					printf("key1 is released!\n");
				}
				break;
			case KEY_DOWN:
				if(ev_button.value)
				{
					printf("key2 is pressed!\n");
				}
				else
				{
					printf("key2 is released!\n");
				}
				break;
			case KEY_LEFT:
				if(ev_button.value)
				{
					printf("key3 is pressed!\n");
				}
				else
				{
					printf("key3 is released!\n");
				}
				break;
			case KEY_RIGHT:
				if(ev_button.value)
				{
					printf("key4 is pressed!\n");
				}
				else
				{
					printf("key4 is released!\n");
				}
				break;
			case KEY_ENTER:
				if(ev_button.value)
				{
					printf("key5 is pressed!\n");
				}
				else
				{
					printf("key5 is released!\n");
				}
				break;
			case KEY_BACK:
				if(ev_button.value)
				{
					printf("key6 is pressed!\n");
				}
				else
				{
					printf("key6 is released!\n");
				}
				break;

			case KEY_HOME:
				if(ev_button.value)
				{
					printf("key7 is pressed!\n");
				}
				else
				{
					printf("key7 is released!\n");
				}
				break;
			case KEY_POWER:
				if(ev_button.value)
				{
					printf("key8 is pressed!\n");
				}
				else
				{
					printf("key8 is released!\n");
				}
				break;
			default:
				break;
		}
	
	}
	return 0;
}


