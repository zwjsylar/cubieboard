#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
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

	//create two socket, msg_socket for the command, data_socket for the data
	if( (msg_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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
	
	while(1)
	 {
		write(msg_socket, "hello world", strlen("hello world"));	
	}
}
