#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>


#define EOL "\r\n"
#define EOL_SIZE 2

typedef struct {
	char *ext;
	char *mediatype;
	} extn;

extn extensions[] = {
	{"htm", "text/html"},
	{"html", "text/html"},
	{0,0} };

void error(const char *msg)
{
	perror(msg);
	exit(1);
}


int get_file_size(int fd)
{
	struct stat stat_struct;
	if(fstat(fd, &stat_struct) == -1)
		return(1);
	return (int) stat_struct.st_size;
}

void send_new(int fd, char *msg)
{
	int len = strlen(msg);
	if(send(fd,msg,len,0) == -1)
	{
		printf("Error in send\n");
	}
}


int recv_new(int fd,char *buffer)
{
	char *p = buffer;
	int matched=0;
	while(recv(fd,p,1,0) != 0)
	{
		if(*p == EOL[matched])
		{
			++matched;
		if(matched == EOL_SIZE) 
		{
			*(p + 1 - EOL_SIZE) = '\0';
			return (strlen(buffer));
		}
	    }
	else
	{
		matched = 0;
	}
		p++;
	}
	return 0;
}

char* webroot() 
{
	FILE *in = fopen("conf", "rt");
	char buff[1000];
	fgets(buff,1000,in);
	fclose(in);

	char *ptr = strrchr(buff,'\n');
	if(ptr != NULL)
		*ptr = '\0';
	return strdup(buff);
}

int connection(int fd)
{
	char request[500],resource[500],*ptr;
	int fd1,length;
	if(recv_new(fd,request) == 0) {
		printf("Recieve Failed\n");
	}

	printf("%s\n",request);

	ptr = strstr(request,"HTTP/");
	if(ptr == NULL)
	{
		printf("NOT HTTP!\n");
	}
	else
	{
		*ptr = 0;
		ptr = NULL;
	
	if(strncmp(request,"GET ",4) == 0)
	{ 
	ptr = request + 4;
	}
	if(ptr == NULL)
	{
		printf("Unknown Request! \n");
	}
	else
	{
		if(ptr[strlen(ptr) -1] =='/')
		{
			strcat(ptr,"index.html");
		}
	strcpy(resource,webroot());
	strcat(resource, ptr);

	char* s = strchr(ptr,'.');
	int i;

	for(i=0; extensions[i].ext != NULL; i++)
 	{
		if( strcmp(s + 1, extensions[i].ext) == 0)
		{
			fd1 = open(resource,O_RDONLY,0);
			printf("Opening \"%s\"\n", resource);
      if (fd1 == -1)
      {	
      printf("404 File not found Error\n");
      send_new(fd, "HTTP/1.1 404 Not Found\r\n");
      send_new(fd, "Server : Web Server in C\r\n\r\n");
      send_new(fd, "<html><head><title>404 Not Found</head></title>");
      send_new(fd, "<body><p>404 Not Found: The requested resource could not be found!</p></body></html>");
      }
	
      else {
      printf("200 OK, Content-Type: %s\n\n",extensions[i].mediatype);
      send_new(fd, "HTTP/1.1 200 OK\r\n");
      send_new(fd, "Server : Web Server in C\r\n\r\n");

      if (ptr == request + 4) 
        {
       if ((length = get_file_size(fd1)) == -1)
	        printf("Error in getting size !\n");

       size_t total_bytes_sent = 0;
       ssize_t bytes_sent;

       while (total_bytes_sent < length) 
	{
        	if ((bytes_sent = sendfile(fd, fd1, 0,length - total_bytes_sent)) <= 0) 
		{
         	if (errno == EINTR || errno == EAGAIN) {
          	continue;
         	}
         perror("sendfile");
         return -1;
        }
        total_bytes_sent += bytes_sent;
       }

      }
     }
     break;
    }

    int size = sizeof(extensions) / sizeof(extensions[0]);
    if (i == size - 2) {
     printf("415 Unsupported Media Type\n");
     send_new(fd, "HTTP/1.1 415 Unsupported Media Type\r\n");
     send_new(fd, "Server : Web Server in C\r\n\r\n");
     send_new(fd, "<html><head><title>415 Unsupported Media Type</head></title>");
     send_new(fd, "<body><p>415 Unsupported Media Type!</p></body></html>");
    }
   }

   close(fd);
  }
 }
 shutdown(fd, SHUT_RDWR);
}

int main(int argc, char *argv[])
{
	int sockfd,newsockfd,portno,pid;
	socklen_t clien;
	struct sockaddr_in serv_addr, cli_addr;

	if(argc < 2)
 	{
		fprintf(stderr, "Error, no port provided\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("Error opening socket");

	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
 	serv_addr.sin_addr.s_addr = INADDR_ANY;
 	serv_addr.sin_port = htons(portno);	
	
	listen(sockfd, 5);
	clien= sizeof(cli_addr);
	
	while(1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clien);
		if(newsockfd < 0)
			error("ERROR on accept");

		pid = fork();
		if(pid < 0)
			error("ERROR on fork");

		if(pid == 0)
		{
			close(sockfd);
			connection(newsockfd);
			exit(0);
		}
		else
			close(newsockfd);
	}
	close(sockfd);
	return 0;
}

