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

#define BUFSIZE 1024

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
void error(char *msg) {
  perror(msg);
  exit(1);
}

int receive_image(int socket, char* fileName)
{ // Start function 

  int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

  char imagearray[10241],verify = '1';
  FILE *image;

  //Find the size of the image
  do{
    stat = read(socket, &size, sizeof(int));
  }while(stat<0);

  printf("Packet received.\n");
  printf("Packet size: %i\n",stat);
  printf("Image size: %i\n",size);
  printf(" \n");

  char buffer[] = "Got it";

  //Send our verification signal
  do{
  stat = write(socket, &buffer, sizeof(int));
  }while(stat<0);

  printf("Reply sent\n");
  printf(" \n");

  image = fopen(fileName, "w");

  if( image == NULL) {
  printf("Error has occurred. Image file could not be opened\n");
  return -1; }

  //Loop while we have not received the entire file yet


  int need_exit = 0;
  struct timeval timeout = {1,0};

  fd_set fds;
  int buffer_fd, buffer_out;

  while(recv_size < size) {
  //while(packet_index < 2){

    FD_ZERO(&fds);
    FD_SET(socket,&fds);

    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);
    if (buffer_fd < 0)
       printf("error: bad file descriptor set.\n");

    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");

    if (buffer_fd > 0)
    {
        do{
            read_size = read(socket,imagearray, 10241);
        }while(read_size <0);

        printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);


        //Write the currently read data into our image file
        write_size = fwrite(imagearray,1,read_size, image);
        printf("Written image size: %i\n",write_size); 

        if(read_size !=write_size) {
          printf("error in read write\n");    }
          //Increment the total number of bytes read
          recv_size += read_size;
          packet_index++;
          printf("Total received image size: %i\n",recv_size);
          printf(" \n");
          printf(" \n");
        }
    }
    fclose(image);
    printf("Image successfully Received!\n");
    return 1;
}

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  fileDetails f;

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
  while (1) {
	  fileDetails f;
    /* 
     * accept: wait for a connection request 
     */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) 
      error("ERROR on accept");
    
    /* 
     * gethostbyaddr: determine who sent the message 
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    
    /* 
     * read: read input string from the client
     */
    bzero(buf, BUFSIZE);
    n = read(childfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
	  else
		  strcpy(f.fileName, buf);
    printf("server received %d bytes: %s\n", n, buf);
    
    /* 
     * write: echo the input string back to the client 
     */
    n = write(childfd, buf, strlen(buf));
    if (n < 0) 
      error("ERROR writing to socket");

    bzero(buf, BUFSIZE);
    n = read(childfd, buf, BUFSIZE);
    
    if (n < 0) 
      error("ERROR reading from socket");
    int read_size = atoi(buf);
    printf("%d\n", read_size);
	  f.fileSize = read_size;
    printf("server received %d bytes: %s\n", n, buf);
    
    /* 
     * write: echo the input string back to the client 
     */
    n = write(childfd, buf, strlen(buf));
    if (n < 0) 
      error("ERROR writing to socket");

    receive_image(childfd, f.fileName);
	
  	char command[1024];
  	strcpy(command, "md5sum ");
  	strcat(command, f.fileName);
  	FILE *md5_cmd = popen(command, "r");
  	if(md5_cmd == NULL){
  		fprintf(stderr, "popen(3) error");
  		exit(EXIT_FAILURE);
  	}
    printf("md5 reached\n");
  	static char buffer[1024];
  	size_t m;
  	while((m=fread(buffer,1,sizeof(buffer)-1, md5_cmd))>0){  
  		buffer[m]='\0';
  		break;
  	}
  	m = write(childfd, buffer, sizeof(buffer));
  	if(m<0)
  		error("ERROR in writing to socket\n");
  }
  close(childfd);
}