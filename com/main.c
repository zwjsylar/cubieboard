#include "comdev.h"

int ComInit(char *dev, int baudrate)
{
    int fd;
    char devname_head[10] = "/dev/";
    char dev_name[20];

    strcpy(dev_name, devname_head);
    strcat(dev_name, *dev);

    fd = open(dev_name, O_RDWR):

    if(fd < 0)
    {
	perror("error to open /dev/ttySx\n");
	exit(1);
    }

    if(fd > 0)
    {
	set_speed(fd, baudrate);
    }
    else
    {
	printf("Can't Open Serial Port!\n");

	exit(0);
    }

    if( set_Parity(fd, 8, 1, 'N') == FALSE)
    {
	printf("Set Parity Erroe\n");

	exit(1);
    }
    printf("\n Open /dev/%s successfully, baudrate is %d\n", *dev, baudrate);
    
    return fd;
}

int main()
{
    printf("hello world\n");
    return 0;
}
