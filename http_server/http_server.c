/** 
* Written by: 莫安德 B96902124 
**/
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define TIMEOUT_SEC 5		// timeout in seconds for wait for a connection 
#ifndef MAXBUFSIZE
#define MAXBUFSIZE  10000	// maximum amount of characters per line and maximum size of connections queued for the socket
#endif

char hostname[512];	//Server's name


//String showing 404 error
const char NOTFOUND_A[] = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The requested URL was not found on this server.</p><p>Better luck next time...</p></body></html>";

//String showing 501 error
const char NOTFOUND_B[] = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>501 Not supported</title></head><body><h1>501 Not Supported</h1><p>The requested method is not supported on this server.</p><p>Better luck next time...</p></body></html>";


struct sockaddr_in servaddr;

//Headers struct
typedef struct{
	char request[MAXBUFSIZE];
	char path[MAXBUFSIZE];
	char protocol[15];
}STATUS;

int server_starter(int port);	//Starts server socket, listen and bind

void written(int fd, char protocol[], int status); //Sends bytes to client

int main(int argc, char **argv){
	STATUS header_stats;
	char client[512];	//Client address
	int connfd, openfd, listenfd;	//bytes and file descriptors
	pid_t childpid;		//Child process
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	struct stat buf;	//To use with stat
	ssize_t read_fd;
	ssize_t read_bytes;
	char line[MAXBUFSIZE];		//line input
	char header[MAXBUFSIZE];	//header input

	char beginning[MAXBUFSIZE];	//includes htdocs string

	unsigned short port;		//Server's id

	if ( argc != 2 ){
		(void) fprintf( stderr, "usage:  %s AND port#\n", argv[0] );
		exit( 1 );
	}

	port = (unsigned short) atoi( argv[1] );

	listenfd = server_starter(port);

	if(listen(listenfd, MAXBUFSIZE) < 0){
		perror("socket");
		exit(1);
	}

	gethostname(hostname, sizeof(hostname) );	//Server's name

	fprintf(stderr, "\nstarting on %.80s, port %d, fd %d...\n", hostname, servaddr.sin_port, listenfd);

	//Main loop to read and write bytes from server to client
	for(;;){
	
		strcpy(beginning, "htdocs");
		
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, (socklen_t *) &clilen);

		if ( connfd < 0 ){
			perror("Connection error!");
			return 1;
		}

		strcpy(client, inet_ntoa(cliaddr.sin_addr));
		fprintf( stderr, "getting a new request from %s\n", client );


		if((read_fd = recv(connfd, line, MAXBUFSIZE, 0)) < 0)
			continue;

		line[read_fd] = '\0'; //Sets the null value for the line string
		
		printf("%s\n", line);

		if ((childpid = fork()) == 0){ //Fork to start working with the requests
		
			//Token used to set the header struct
			strcpy(header, line);
			strcpy(header_stats.request, strtok(header, " \n\r\t"));
			strcpy(header_stats.path, strtok(NULL, " \n\r\t"));
			strcpy(header_stats.protocol, strtok(NULL, " \n\r\t"));

			if((!header_stats.request) || (!header_stats.path) || (!header_stats.protocol))
				return -1;

			strcat(beginning, header_stats.path);

			//Check if the path exists
			if(stat(beginning, &buf) < 0){
				if (errno == ENOENT){
				 	written(connfd, header_stats.protocol, 1);
				}
				close(connfd);
				return -1;
			}

			//Check if the path is a directory
			if(S_ISDIR (buf.st_mode)){
				if(beginning[strlen(beginning) - 1] == '/'){
					strcat(beginning, "index.html");
					}
				else{
					strcat(beginning, "/index.html");
				}
			}

			//Open file
			openfd = open(beginning, O_RDONLY);

			//if the requirement is GET we print the whole file
			if(strcmp(header_stats.request, "GET") == 0){

				if (openfd < 0){ 
					written(connfd, header_stats.protocol, 1);
					break;
				}
				else{
					written(connfd, header_stats.protocol, 0);
					while((read_bytes = read(openfd, line, MAXBUFSIZE)) > 0){
						send(connfd, line, read_bytes, 0);
					}
				}
			}
			//if the requirement is HEAD we just print the first line
			else if (strcmp(header_stats.request, "HEAD") == 0){
				if (openfd < 0){ 
					written(connfd, header_stats.protocol, 1);
				}
				else{
					written(connfd, header_stats.protocol, 0);
				}
			}
			//if the requirement is POST we just print the line string
			else if (strcmp(header_stats.request, "POST") == 0){      
				printf("%s\n",line);
			}       
			//Error 501
			else{
				written(connfd, header_stats.protocol, 3);
				return -1;
				
			}
			
			exit(0);
		}		
		
		close(connfd);

	}	
	return 0;
}

//Start server}
int server_starter(int port){
	int listenfd;
	int tmp;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd < 0){
		perror("Error opening socket");
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	tmp = 1;

	//Set socket options: Allow reuse of local addresses
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*) &tmp, sizeof(tmp)) < 0){
		perror("Error setting options for the socket");
		exit(1);
	}

	//Defines the port we are using
	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) < 0){
		perror("Error ");
		exit(1);
	}

	return listenfd;	
}

//Write bytes to client
void written(int fd, char protocol[], int status){
	char link_status[64] = "200 OK";
	//CRLF at the end of the header.
	char CRLF[3];
	CRLF[0] = 0x0D;
	CRLF[1] = 0x0A;
	CRLF[2] = '\0';
	send(fd, protocol, strlen(protocol), 0);
	send(fd, " ", 1,0);
	if (status == 1)
	strcpy(link_status, "404 Not Found");
	send(fd, link_status, strlen(link_status), 0);
	send(fd, CRLF, strlen(CRLF), 0);
	send(fd, CRLF, strlen(CRLF), 0);
	if (status == 1)
	send(fd, NOTFOUND_A, strlen(NOTFOUND_A), 0);
	if (status == 3)
	send(fd, NOTFOUND_B, strlen(NOTFOUND_B), 0);
}
