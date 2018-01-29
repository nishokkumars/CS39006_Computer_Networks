/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port> <fileName>
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h> 
#define BUFSIZE 1024 // to divide into chunks of 1 KB
#define h_addr h_addr_list[0]
/* 
 * error - wrapper for perror
 */
typedef struct {
   
   char fileName[512];
   int fileSize;
   int noOfChunks;

} fileDetails;

typedef struct {
 
   char chunkContents[1024];
   int sequenceNumber;
   int chunkLength;

} fileChunk;

void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char *fileName;
    char buf[BUFSIZE];
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    /* check command line arguments */
    if (argc != 4) {
       fprintf(stderr,"usage: %s <hostname> <port> <filename>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    fileName = argv[3];

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
    bcopy((char *)server->h_addr,(char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    bzero(buf, BUFSIZE);
    
    //printf("Please enter msg: ");
    //fgets(buf, BUFSIZE, stdin);
    FILE *fp = fopen(argv[3],"r");
    if (fp == NULL) {
        printf("File does not exist \n");
        return 1;
    }
    fseek(fp, 0, 2);    /* file pointer at the end of file */
    int fileSize = ftell(fp); 
    fclose(fp);
    fp = fopen(argv[3],"r");

    int noOfChunks = fileSize%1024 == 0 ? fileSize/1024 : (fileSize/1024+1);
    
    fileDetails f;
    strcpy(f.fileName,argv[3]);
    f.fileSize = fileSize;
    f.noOfChunks = noOfChunks;
    printf("%s %d %d\n",f.fileName,f.fileSize,f.noOfChunks);
    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, (char*)(&f), sizeof(f), 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    
    /* print the server's reply */
    n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
    fclose(fp);
    return 0;
}
