#ifndef COMINIT_H_INCLUDED
#define COMINIT_H_INCLUDED

extern int ComInit(char *dev, int baudrate);
extern int addFdToEpfd(int epfd, int fd);

#endif // COMINIT_H_INCLUDED
