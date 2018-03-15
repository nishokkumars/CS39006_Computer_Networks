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

typedef struct{
  
   int sequenceNumber;
   int chunkLength;
   int recv_window_left;

}header;

typedef struct{
 
   header packetHeader;
   char packetContents[1024];

}dataPacket;
 
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
  if (argc != 3) {
    fprintf(stderr, "usage: %s <port_for_server> <drop-probability>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);
  double drop_probability = atof(argv[2]);

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
    
    srand(time(NULL));

    dataPacket p;
    //printf("%d %d",noOfReceivedPackets,f.noOfChunks);
    FILE* fp = fopen("test.cpp","wb");
    int expectedseqnum = 1;
    int prevAck = 0;
    while(expectedseqnum<=5)
    {
         n = recvfrom(sockfd, (char*)&p, sizeof(p), 0,(struct sockaddr *) &clientaddr, &clientlen);
         puts(p.packetContents);
         int seqNo = p.packetHeader.sequenceNumber;
         printf("%d\n",seqNo);
         if(seqNo == expectedseqnum)
         {
            unsigned int i;
            for(i = 0; i < p.packetHeader.chunkLength; ++i) {
                fputc(p.packetContents[i],fp);
            }
            int t = drop_probability*1000;
            int no = rand()%1000;
            if(no<=t)
            {
              n = sendto(sockfd, (char*)&expectedseqnum, sizeof(int), 0,(struct sockaddr *) &clientaddr, clientlen);
            }  
            prevAck = expectedseqnum;
            expectedseqnum++;
         }else{

            int t = drop_probability*1000;
            int no = rand()%1000;
            if(no<=t)
            {
              n=sendto(sockfd, (char*)&prevAck,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
            }
         }
     }
     fclose(fp);
     char command[1024];
     strcpy(command,"md5sum ");
     strcat(command,"test.cpp");
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
