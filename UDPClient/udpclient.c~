/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port> <fileName>
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h> 
#include <poll.h>
#include <stdbool.h>
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
    struct addrinfo *my_info,hints;
    struct hostent *server;
    char *hostname;
    char *fileName;
    char buf[BUFSIZE];

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
   
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, argv[2], &hints, &my_info);

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);



    /* get a message from the user */
    bzero(buf, BUFSIZE);
    
    //printf("Please enter msg: ");
    //fgets(buf, BUFSIZE, stdin);
    FILE *fp = fopen(argv[3],"rb");
    if (fp == NULL) {
        printf("File does not exist \n");
        return 1;
    }
    struct stat inputFileInfo;

    // Get filesize to calculate total number of fragments and total number of fragment digits
    stat(argv[3], &inputFileInfo);
    int fileSize = inputFileInfo.st_size;
    
    int noOfChunks = fileSize%1024 == 0 ? fileSize/1024 : (fileSize/1024+1);
    
    fileDetails f;
    strcpy(f.fileName,argv[3]);
    f.fileSize = fileSize;
    f.noOfChunks = noOfChunks;
    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, (char*)(&f), sizeof(f), 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    int temp=f.fileSize;
    /* print the server's reply */
    char buff[1024];
    n = recvfrom(sockfd, buff, 1024, 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
    printf("Message from server: %s \n",buff);
    int noOfSentPackets = 0;
    int prevRead = 1;
    fileChunk p;
    while(noOfSentPackets!=f.noOfChunks)
    {
         if(prevRead)
         { 
            unsigned int i;char nextChar;
            bool endOfFile = feof(fp);
            for (i = 0; i < BUFSIZE && !endOfFile; i++) {
                nextChar = getc (fp);
                if (feof(fp)) {
                    endOfFile = true;
                    i--;
                } else {
                    p.chunkContents[i] = nextChar;
                }
            }
            p.chunkContents[i]='\0';
            p.sequenceNumber = noOfSentPackets+1;
            p.chunkLength = i;
            prevRead = 0;
            
         }
         n = sendto(sockfd, (char*)(&p), sizeof(p), 0, &serveraddr, serverlen);
         if (n < 0) 
           error("ERROR in sendto");
         char buf3[50];
         bzero(buf3,50);
                  struct pollfd fd;
         int ret;

           fd.fd = sockfd; // your socket handler 
           fd.events = POLLIN;
            ret = poll(&fd, 1, 1000); // 1 second for timeout
          switch (ret) {
          case -1:
             // Error
            break;
          case 0:
          prevRead=0;
          break;
          default:
         n = recvfrom(sockfd, buf3, 15, 0, &serveraddr, &serverlen);
         if (n < 0) 
            error("ERROR in recvfrom");
         char buf1[50]="ACK";
         char buf2[10];
         sprintf(buf2,"%d",noOfSentPackets+1);
         strcat(buf1,buf2);
         printf("Message from server: %s\n",buf3);
         if(strcmp(buf1,buf3)==0)
         {
            noOfSentPackets++;
            prevRead = 1;
         } // get your data
        break;
         }

    }
    char buffer[1024];
    n = recvfrom(sockfd, buffer, 1024, 0, &serveraddr, &serverlen);
    char command[1024];
     strcpy(command,"md5sum ");
     strcat(command,f.fileName);
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
    freeaddrinfo(my_info);
    return 0;
}
