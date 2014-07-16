/*********************************************************************
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>


int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
        B38400, B19200, B9600, B4800, B2400, B1200, B300, };

int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
        38400,  19200,  9600, 4800, 2400, 1200,  300, };


void set_speed(int fd, int speed)
{
  int i;
  int status;

  struct termios Opt;
  tcgetattr(fd,&Opt);

  for (i= 0;i<sizeof(speed_arr)/sizeof(int);i++)
  {
        if(speed == name_arr[i])
       {
           tcflush(fd, TCIOFLUSH);


        cfsetispeed(&Opt, speed_arr[i]);

        cfsetospeed(&Opt, speed_arr[i]);


        status = tcsetattr(fd, TCSANOW, &Opt);

        if(status != 0)
           perror("tcsetattr fd1");

             return;
         }
        tcflush(fd,TCIOFLUSH);
   }
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
   if( tcgetattr( fd,&options)!= 0)
   {
          perror("SetupSerial 1");

           return(FALSE);
    }

   options.c_cflag &= ~CSIZE;


  switch(databits)
   {
          case 7:
          options.c_cflag |= CS7;
          break;

       case 8:
        options.c_cflag |= CS8;
        break;

       default:
        fprintf(stderr,"Unsupported data size\n");

        return (FALSE);
    }

   switch(parity)
      {
          case 'n':
        case 'N':
        options.c_cflag &= ~PARENB;    /* Clear parity enable */
        options.c_iflag &= ~INPCK;    /* Enable parity checking */
        options.c_iflag &= ~(ICRNL|IGNCR);
        options.c_lflag = 0;
        break;


    case 'o':

    case 'O':
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;    /* Disnable parity checking */
        break;


    case 'e':

    case 'E':
        options.c_cflag |= PARENB;    /* Enable parity */
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;    /* Disnable parity checking */
        break;


    case 'S':

    case 's':  /*as no parity*/
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;

         default:
        fprintf(stderr,"Unsupported parity\n");

        return (FALSE);
    }

   switch(stopbits)
      {

      case 1:

      options.c_cflag &= ~CSTOPB;

    break;


    case 2:

    options.c_cflag |= CSTOPB;

    break;


    default:

    fprintf(stderr,"Unsupported stop bits\n");


    return (FALSE);
    }

  /* Set input parity option */

   if(parity != 'n')
          options.c_iflag |= INPCK;
          options.c_cc[VTIME] = 150; // 15 seconds
       options.c_cc[VMIN] = 0;

         tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */

  if(tcsetattr(fd,TCSANOW,&options) != 0)
  {
          perror("SetupSerial 3");

        return (FALSE);
    }

   return (TRUE);
}

int ComInit(char *dev, int baudrate)
{
    int fd;
    char devname_head[10] = "/dev/";
    char dev_name[20];

    strcpy(dev_name, devname_head);
    strcat(dev_name, dev);

    fd = open(dev_name, O_RDWR);

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
    printf("\n Open /dev/%s successfully, baudrate is %d\n", dev, baudrate);

    return fd;

}

/*int init_serial(char *dev)
{
    int serial_fd;
    char devname_head[10] = "/dev/";
    char dev_name[20];

    strcpy(dev_name, devname_head);
    strcat(dev_name, dev);

    serial_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd < 0) {
        perror("open");
        return -1;
    }
    struct termios options;
    tcgetattr(serial_fd, &options);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CSTOPB;
    options.c_iflag |= IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    cfsetospeed(&options, B9600);
    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &options);

    return serial_fd;
}
*/
int addFdToEpfd(int epfd, int fd)
{
    struct epoll_event event;
    int flags;

    //set it unblock
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    event.events = EPOLLOUT | EPOLLIN | EPOLLET; // set it to read
    event.data.fd = fd;

    if( epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) )	//epoll_ctl:: if succeed ,return 0; if failed, return -1
    {
        perror("epoll_ctl(ADD) error\n");
        return -1;
    }
    else return 0;
}
