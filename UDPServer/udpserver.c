/* 
 * udpserver.c - A UDP echo server 
 * usage: udpserver <port_for_server>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>


#define BUFSIZE 1024 // to divide into chunks of 1 KB
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
  exit(1);
}

int main(int argc, char **argv) {
  int sockfd; /* socket file descriptor - an ID to uniquely identify a socket by the application program */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port_for_server>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));


  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
    /*
     * recvfrom: receive a UDP datagram from a client
     */

  while(1)
    {
    fileDetails f;
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, (char*)&f, sizeof(fileDetails), 0,(struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");
    int noOfReceivedPackets = 0;
    int packetsReceived[f.noOfChunks+4];
    int i;
    for(i=1;i<=f.noOfChunks;++i)
      packetsReceived[i]=0;
    

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", sizeof(fileDetails), f.noOfChunks, f.fileName);
   
    /* 
     * sendto: echo the input back to the client 
     */
    bzero(buf,BUFSIZE);
    strcpy(buf,"Received file details");
    printf("%s %d %d\n",buf,f.fileSize,f.noOfChunks);
    n = sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
    fileChunk allPackets[f.noOfChunks+4];
    fileChunk p;
    //printf("%d %d",noOfReceivedPackets,f.noOfChunks);
    FILE* fp = fopen(f.fileName,"wb");
    while(noOfReceivedPackets<f.noOfChunks)
    {
         n = recvfrom(sockfd, (char*)&p, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, &clientlen);
         int seqNo = p.sequenceNumber;
         if(packetsReceived[seqNo])
         {
            memset(buf,0,strlen(buf));
            strcpy(buf,"ACK");
            char seqId[10];
            sprintf(seqId,"%d",seqNo);
            strcat(buf,seqId);

           n = sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *) &clientaddr, clientlen);
           if (n < 0) 
             error("ERROR in sendto");
        }
         else
         {
             noOfReceivedPackets++;
             hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
             if (hostp == NULL)
             error("ERROR on gethostbyaddr");
             hostaddrp = inet_ntoa(clientaddr.sin_addr);
             if (hostaddrp == NULL)
             error("ERROR on inet_ntoa\n");
             printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
             printf("server received %d/%d bytes: SequenceNumber:%d\n", sizeof(fileChunk), n, p.sequenceNumber);
             unsigned int i;
             for(i = 0; i < p.chunkLength; ++i) {
                fputc(p.chunkContents[i],fp);
             }
             packetsReceived[seqNo]=1;
             char buf1[20];
             bzero(buf1,20);
             strcpy(buf1,"ACK");
             char seqId[10];
             sprintf(seqId,"%d",seqNo);
             strcat(buf1,seqId);
             buf1[strlen(buf1)]='\0';
             n = sendto(sockfd, buf1, strlen(buf1), 0,(struct sockaddr *) &clientaddr, clientlen);
             if (n < 0) 
                error("ERROR in sendto");
         }
     }
     fclose(fp);
     char command[1024];
     strcpy(command,"md5sum ");
     strcat(command,f.fileName);
    FILE *md5_cmd = popen(command, "r");
    if (md5_cmd == NULL) {
        fprintf(stderr, "popen(3) error");
        exit(EXIT_FAILURE);
    }

    static char buffer[1024];
    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer)-1, md5_cmd)) > 0) {
        buffer[n] = '\0';
        break;
    }
    n = sendto(sockfd, buffer, strlen(buffer), 0,(struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
        error("ERROR in sendto");


    }

  return 0;
}
