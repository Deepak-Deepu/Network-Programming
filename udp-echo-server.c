#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>

#define MAXLINE 1024

int main(int arg,char **argv)
{
	int sockfd;
	int n;
	socklen_t len;
	char msg[1024];
	struct sockaddr_in servaddr,cliaddr;

	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(5045);

	printf("\n\n Binded");
	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	printf("\n\n Listening..");

	for(;;)
	{
		printf("\n");
		len=sizeof(cliaddr);
		n = recvfrom(sockfd,msg,MAXLINE,0,(struct sockaddr *)&cliaddr,&len);
		printf("\n Client Message:%s\n",msg);
		if(n < 6)
			perror("send error");
		sendto(sockfd,msg,n,0,(struct sockaddr *)&cliaddr,len);
	}
	return 0;
}
	
