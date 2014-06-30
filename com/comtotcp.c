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
#define TRUE 1
#define FALSE -1

int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
        B38400, B19200, B9600, B4800, B2400, B1200, B300, };

int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
        38400,  19200,  9600, 4800, 2400, 1200,  300, };

const char SendData[8] = {0x01, 0x04, 0x13, 0x87, 0x00, 0x45, 0x85, 0x54};

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

int init_serial(char *dev)
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
/*inout:    epfd:   the epoll
	    fd:	    the fd need to be read
	    *buf:   the data
	    length: data length
 ouput:	    the length of data

int rev_data(int epfd, int fd, char *buf, int length)
{
    int nread;

    memset(buf, 0, length);

    nread = read(fd, buf, length);

    return nread;

}
*/

int main(int argc, char **argv)
{
    int fd;
    int epfd;
    struct epoll_event events, ev;
    int nread;
    int nwrite;
    int ret;
    int i;
    char rev_buffer[15] ={0};
//    char SendData[8] = {0x01, 0x04, 0x13, 0x87, 0x00, 0x45, 0x85, 0x54};

    epfd = epoll_create(1);
    if( epfd < 0 )
    {
        perror("epoll_create error");
        return -1;
    }

//    fd = init_serial(argv[3]);
    fd  = ComInit(argv[3], 9600);
    addFdToEpfd(epfd, fd);
    printf("\nWelcome to uart_test\n\n");

    memset(rev_buffer,0,sizeof(rev_buffer));

    int msg_socket, data_socket;
    struct hostent *host;
    struct sockaddr_in msg_addr, data_addr;

    if( argc < 3 ){
        fprintf(stderr, "Please enter the server's hostname and port\n");
        exit(1);
        }
    //get the ip address
    if((host = gethostbyname(argv[1])) == NULL ){
        perror("gethostname error\n");
        exit(1);
        }

    if((msg_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("msg_socket error\n");
        exit(0);
        }

    msg_addr.sin_family = AF_INET;
    msg_addr.sin_port = htons(atoi(argv[2]));
    memcpy((char*)&msg_addr.sin_addr, (char*)host->h_addr, host->h_length);

    //connnet to the server
    if( connect(msg_socket, (struct sockaddr *)&msg_addr, sizeof msg_addr) == -1){
        perror("msg_socket connect error\n");
        exit(1);
        }

   //char test[15] = "hello world";
    while(1)
    {
        printf("waitint for data\n");
        ret = epoll_wait(epfd, &events, 1, -1);
        printf("wait for data success\n");
        if(ret <= 0)
        {
            perror("epoll_wait");
            return -1;
        }

        for( i = 0; i < ret; i++)
        {
            if( events.events & EPOLLOUT )
                {
                    printf("send data\n");
                    int temp_fd;
                    temp_fd = events.data.fd;

                    if ((nwrite = write(temp_fd, SendData, 8)) < 0)
                        perror("com write");
                    printf("send %d data success\n", nwrite);
                    ev.data.fd = temp_fd;
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, temp_fd, &ev);
                }

            else if(events.events & EPOLLIN)
            {
                printf("read data\n");
                int temp_fd = events.data.fd;
                if ((nread = read(temp_fd, rev_buffer, sizeof(rev_buffer)-2)) < 0)
                    perror("com read");
                rev_buffer[nread] = '\0';
                if ((nwrite = write(msg_socket, rev_buffer, nread)) < 0)
                    perror("msg_socket write");
                printf("read data :%s\n", rev_buffer);
                ev.data.fd = temp_fd;
                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_MOD, temp_fd, &ev);
            }
            else
            {
            }
        }
        sleep(1);
    }
    return 0;
}

