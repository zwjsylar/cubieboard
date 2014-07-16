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
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <pthread.h>
#include <signal.h>
#include "ComInit.h"

int bt_socket;
void *bt_pthread(void *arg);
static void sig_usr(int socket);

const char SendData[8] = {0x01, 0x04, 0x13, 0x87, 0x00, 0x45, 0x85, 0x54};

struct BtData
{
    char BtDataBuf[1024];
    int BtSize;
    pthread_rwlock_t BtLock;
};

struct BtData bt_data;

int BtData_init(struct BtData *BD)
{
    int err;
    (*BD).BtSize = 0;
    memset((*BD).BtDataBuf,0,sizeof((*BD).BtDataBuf));
    err = pthread_rwlock_init(&(*BD).BtLock, NULL);
    if (err != 0)
        return err;
    return 0;
}

int main(int argc, char **argv)
{
    int ComFd;
    int epfd;
    struct epoll_event events, ev;
    int nread;
    int nwrite;
    int ret;
    int i;

    int BtData_status;
    BtData_status = BtData_init(&bt_data);
//    char SendData[8] = {0x01, 0x04, 0x13, 0x87, 0x00, 0x45, 0x85, 0x54};

//creat pthread
    int bt_status;
    pthread_t bt_tid;
    bt_status = pthread_create(&bt_tid, NULL, bt_pthread, NULL);

//creat epoll
    epfd = epoll_create(1);
    if( epfd < 0 )
    {
        perror("epoll_create error");
        return -1;
    }

//    init com fd
    ComFd  = ComInit(argv[3], 9600);
    addFdToEpfd(epfd, ComFd);
    printf("\nWelcome to uart_test\n\n");


// init socket fd
    int msg_socket, data_socket;
    struct hostent *host;
    struct sockaddr_in msg_addr, data_addr;
    if( argc < 3 )
    {
       fprintf(stderr, "Please enter the server's hostname and port\n");
       exit(1);
    }
    //get the ip address
    if((host = gethostbyname(argv[1])) == NULL )
    {
        perror("gethostname error\n");
        exit(1);
    }
    if((msg_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("msg_socket error\n");
        exit(0);
    }
    msg_addr.sin_family = AF_INET;
    msg_addr.sin_port = htons(atoi(argv[2]));
    memcpy((char*)&msg_addr.sin_addr, (char*)host->h_addr, host->h_length);
    //connnet to the server
    if( connect(msg_socket, (struct sockaddr *)&msg_addr, sizeof msg_addr) == -1)
    {
        perror("msg_socket connect error\n");
        exit(1);
    }

    while(1)
    {
        printf("waiting for data\n");
        ret = epoll_wait(epfd, &events, 1, 10000);
        printf("wait data success\n");
        if(ret < 0)
        {
            perror("epoll_wait");
            break;
        }
        else if(ret == 0)
        {
            ev.data.fd = ComFd;
            ev.events = EPOLLOUT | EPOLLET;
            epoll_ctl(epfd, EPOLL_CTL_MOD, ComFd, &ev);
            continue;
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
                pthread_rwlock_wrlock(&bt_data.BtLock);
                if ((nread = read(temp_fd, bt_data.BtDataBuf, sizeof(bt_data.BtDataBuf)-2)) < 0)
                    perror("com read");
                bt_data.BtDataBuf[nread] = '\0';
                bt_data.BtSize = nread;
                if ((nwrite = write(msg_socket, bt_data.BtDataBuf, nread)) < 0)
                    perror("msg_socket write");
                printf("read data :%s\n", bt_data.BtDataBuf);
                pthread_rwlock_unlock(&bt_data.BtLock);
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
    close(epfd);
    close(ComFd);
    return 0;
}

void *bt_pthread(void *arg)
{
    struct sockaddr_rc loc_addr ={0},rem_addr={0};
    char buf[1024] ={0};//,*addr;
    int client, bytes_write, result;
    int opt = sizeof(rem_addr);


    printf("Creating socket...\n");

    bt_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    signal(SIGINT, sig_usr);
    if(bt_socket < 0)
    {
        perror("create socket error");
        exit(1);
    }
    else
    {
        printf("success!\n");
    }

    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 14;

    printf("Binding socket...\n");
    result = bind(bt_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    if(result<0)
    {
        perror("bind socket error:");
        exit(1);
    }
    else
    {
        printf("success!\n");
    }
	
   /*result=ba2str(&loc_addr.rc_bdaddr,addr);
   if(result<0)
     {
       perror("addr convert error");
       exit(1);
     }
     printf("local addr is:%s/n",addr);*/
    printf("Listen... ");
    result = listen(bt_socket, 1);
    if(result<0)
    {
        printf("error:%d\n:",result);
        perror("listen error:");
        exit(1);
    }
    else
    {
        printf("requested!\n");
    }
    while(1)
    {
        printf("Accepting...\n");
        client = accept(bt_socket, (struct sockaddr *)&rem_addr, &opt);
        if(client<0)
        {
            perror("accept error");
            exit(1);
        }
        else
        {
            printf("OK!\n");
        }
        ba2str(&rem_addr.rc_bdaddr, buf);
        fprintf(stderr, "accepted connection from %s \n", buf);
        memset(buf, 0, sizeof(buf));

        while(1)
        {
            if( pthread_rwlock_rdlock(&bt_data.BtLock) != 0)
                continue;
            bytes_write = write(client, bt_data.BtDataBuf, bt_data.BtSize);
            if(bytes_write != bt_data.BtSize)
            {
                printf("bluetooth write error\n");
                exit(1);
            }
            pthread_rwlock_unlock(&bt_data.BtLock);
	    sleep(5);	
	 }
        close(client);
    }
    close(bt_socket);
    return ((void *)0);
}

static void sig_usr(int signo)
{
    close(bt_socket);
}
