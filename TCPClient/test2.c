/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

#define BUFSIZE 1024



/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    fprintf(stderr, "Cannot determine size of %s: %s\n",
            filename, strerror(errno));

    return -1;
}

int send_image(int socket, char * filename){

   FILE *picture;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
   packet_index = 1;

   picture = fopen(filename, "r");
   printf("Getting Picture Size\n");   

   if(picture == NULL) {
        printf("Error Opening Image File"); } 

   fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   printf("Total Picture size: %i\n",size);

   //Send Picture Size
   printf("Sending Picture Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   printf("Sending Picture as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat=read(socket, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   printf("Received data in socket\n");
   printf("Socket data: %c\n", read_buffer);

   while(!feof(picture)) {
   //while(packet_index = 1){
      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

      //Send data through our socket 
      do{
        stat = write(socket, send_buffer, read_size);  
      }while (stat < 0);

      printf("Packet Number: %i\n",packet_index);
      printf("Packet Size Sent: %i\n",read_size);     
      printf(" \n");
      printf(" \n");
      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
    }
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 4) {
       fprintf(stderr,"usage: %s <hostname> <port> <filename>\n", argv[0]);
       exit(0);
    }
    //if (argc != 4) {
    //   fprintf(stderr,"Enter filename\n", argv[0]);
    //   exit(0);
    //}
    hostname = argv[1];
    portno = atoi(argv[2]);
    char *filename = argv[3];

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

    //char * filename = "testfile2.pdf";
    struct stat inputFileInfo;

    // Get filesize to calculate total number of fragments and total number of fragment digits
    stat(argv[3], &inputFileInfo);
    int size = inputFileInfo.st_size;

    /* get message line from the user */
    //printf("Please enter msg: ");
    //bzero(buf, BUFSIZE);
    //fgets(buf, BUFSIZE, stdin);

    /* send the message line to the server */
    n = write(sockfd, filename, strlen(filename));
    if (n < 0) 
      error("ERROR writing to socket");

    /* print the server's reply */
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    printf("Echo from server: %s\n", buf);

    //printf("%d", size);
    
    char file_size[10];
    snprintf(file_size,sizeof(file_size), "%d",size);
    n= write(sockfd, file_size, strlen(file_size));
    if (n < 0) 
      error("ERROR writing to socket");
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    printf("Echo from server: %s\no", buf);

    int x = send_image(sockfd, filename);
	char buffer[1024];
	n = read(sockfd, buffer, sizeof(buffer));
	char command[1024];
	strcpy(command,"md5sum ");
	strcat(command,filename);
	FILE *md5_cmd = popen(command, "r");
	if (md5_cmd == NULL) {
        fprintf(stderr, "popen(3) error");
        exit(EXIT_FAILURE);
    }
	static char buffer1[1024];
    size_t nn;
	while ((nn = fread(buffer1, 1, sizeof(buffer1)-1, md5_cmd)) > 0) {
        buffer1[nn] = '\0';
        break;
    }
	printf("%s\n",buffer1);
    printf("%s\n",buffer);
    if(strcmp(buffer1,buffer)==0)
    {
        printf("MD5 matched\n");
    }
    else printf("MD5 no match\n");
    close(sockfd);
    return 0;
}
