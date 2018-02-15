/* 
 * tcpserver.c - A simple TCP echo server 
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define BUFSIZE 1024
#define MAXCLIENTS 5

typedef struct {
   
   char fileName[512];
   int fileSize;
   int noOfChunks;

} fileDetails;


#if 0
/* 
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */

typedef struct 
{
  char *name;
  char *ip;
  int port_no;
}user_info;

user_info users[5];
users[0].name = (char *)"subham";
users[0].ip = (char *)"10.147.115.60";
users[0].port_no = 8085;



void error(char *msg) {
  perror(msg);
  exit(1);
}
int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr,serveraddr1; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  fileDetails f;
  fd_set master;
  int i;
  int client_socket[5]={0};
  /* add stdin and the sock fd to master fd_set */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");
  printf("Server Running ....\n");
  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  int max_fd,fd;
  while(1)
  {
    struct timeval t;
    t.tv_sec = 100;
    t.tv_usec =0;
    FD_ZERO(&master);
    //add master socket to set
    FD_SET(parentfd, &master);
    FD_SET(STDIN_FILENO,&master);
    max_fd = parentfd;
    //add child sockets to set
    for ( i = 0 ; i < MAXCLIENTS ; i++) 
    {
         //socket descriptor
         fd = client_socket[i];
         //if valid socket descriptor then add to read list
         if(fd > 0)
            FD_SET( fd , &master);
            //highest file descriptor number, need it for the select function
            if(fd > max_fd)
                max_fd = fd;
    }
    //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
    int activity = select( max_fd + 1 , &master , NULL , NULL ,&t);
    if ((activity < 0) && (errno!=EINTR)) 
    {
            printf("select error");
    }
    for (i= 0; i < MAXCLIENTS; i++) {
        
        fd = client_socket[i];
        .
        if (FD_ISSET(fd, &master)) {
            if (fd == STDIN_FILENO) {
                 
                 bzero(buf,BUFSIZE);
                 scanf(" %[^\n]s",buf);
                 printf("hello\n");
                 printf("%s\n", buf);
                 char* hostname = strtok(buf,"/");
                 char* msg = strtok(NULL,"\n");
                 int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                 if (sockfd < 0) 
                    error("ERROR opening socket");
                 /* gethostbyname: get the server's DNS entry */
                /*struct pollfd timer;
        		timer.fd = sockfd;
        		timer.events = POLLIN;
            	ret = poll(&fd, 1, 1000); // 1 second for timeout*/
            	for(int i=0;i<5;i++){
            		if(strcmp(users[i].name, hostname)==0){
            			hostname = users[i].ip;
            			break;
            		}
            	}
                 struct hostent* server = gethostbyname(hostname);
                 if (server == NULL) {
                      fprintf(stderr,"ERROR, no such host as %s\n", hostname);
                      exit(0);
                 }
                 /* build the server's Internet address */
                 bzero((char *) &serveraddr1, sizeof(serveraddr1));
                 serveraddr1.sin_family = AF_INET;
                 bcopy((char *)server->h_addr, (char *)&serveraddr1.sin_addr.s_addr, server->h_length);
                 serveraddr1.sin_port = htons(portno);
                 /* connect: create a connection with the server */
                 if (connect(sockfd, &serveraddr1, sizeof(serveraddr1)) < 0) 
                       error("ERROR connecting");
                 write(sockfd,msg,strlen(msg));

            }
            else if (fd == parentfd) {
                
                childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
                if (childfd < 0) 
                    error("ERROR on accept");
                //add new socket to array of sockets
                for (i = 0; i < MAXCLIENTS; i++) 
                {
                  //if position is empty
                  if( client_socket[i] == 0 )
                  {
                    client_socket[i] = childfd;

                    printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , childfd , inet_ntoa(clientaddr.sin_addr) , ntohs(clientaddr.sin_port));
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                  }
                }
            }
            else {
                
                int n = read(fd,buf,BUFSIZE);
                buf[n] = '\0';
                printf("Message :");
                printf("%s\n",buf);
            }
        }
    }
  }
  return 0;
}
