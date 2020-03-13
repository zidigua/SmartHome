#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>


#define ERR_PRINT(msg) do{perror(msg);return -1;}while(0)

int sockfd;
struct sockaddr_in clientaddr;

int init_tcp(const char *ip,const char *port)
{
	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
		ERR_PRINT("socket create");

	memset(&clientaddr,0,sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(ip);
	clientaddr.sin_port = htons(atoi(port));

	if(connect(sockfd,(struct sockaddr *)&clientaddr,sizeof(clientaddr)) == -1)
		ERR_PRINT("connect");
	
	return 0;
}

int tcp_send(char *buf)
{
	if(send(sockfd,buf,3,0) == -1)
		ERR_PRINT("tcp send error");
	return 0;
}

int tcp_recv(char *buf)
{
 	if(recv(sockfd,buf,4096,0) == -1)
		ERR_PRINT("tcp recv error");
	return 0;
}

void tcp_close()
{
	close(sockfd);
}
