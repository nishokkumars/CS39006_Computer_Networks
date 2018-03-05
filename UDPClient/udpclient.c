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
  
   int sequenceNumber;
   int chunkLength;

} header;

typedef struct {
 
   header chunkHeader;
   char chunkContents[1024];


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
    struct timeval timeout = {1,0};
    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
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
    fileChunk* filePackets = (fileChunk*)calloc(f.noOfChunks+2,sizeof(fileChunk));
    bool endOfFile = feof(fp);
    int k,i;
    for(k=1;k<=f.noOfChunks && !endOfFile;k++)
    {
       for(i=0;i<BUFSIZE && !endOfFile ; i++)
       {
           char nextChar = getc (fp);
           if (feof(fp)) {
                    
               endOfFile = true;
               i--;
          } else {
              
              filePackets[k].chunkContents[i] = nextChar;
          
          }       	   
       
       }
       filePackets[k].chunkContents[i]='\0';
       filePackets[k].chunkHeader.sequenceNumber = k;
       filePackets[k].chunkHeader.chunkLength = i;
    }
    int base = 1;
    int nextseqnum = 1;
    int N = 3;
    int prevReceived = 0;
    while(base<=f.noOfChunks)
    {
         while(nextseqnum<base+N){
         	
         	n = sendto(sockfd, (char*)(&filePackets[nextseqnum]), sizeof(fileChunk), 0, &serveraddr, serverlen);
            if(base == nextseqnum){

            	//start timer
            }
            nextseqnum++;
         }
         int ack;

         n = recvfrom(sockfd,(char*)&ack,sizeof(int),0,&serveraddr,&serverlen);
         if (n <= 0) {
            
            //start timer
         	int temp;
         	for(temp=base;temp<nextseqnum;temp++)
         	{
         		n = sendto(sockfd,(char*)(&filePackets[temp]),sizeof(fileChunk),0,&serveraddr,serverlen);
         	}
         }
         printf("Message from server: ACK%d\n",ack);
         base = ack + 1;
         N += ack-prevReceived;
         prevReceived = ack;
         if(base == nextseqnum )
         {
            	 N*=2;//stop timer
         }else{
            	 
            	 N/=2;//start timer
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
    char* tokenPtr = strtok(buffer, " ,\t\n"); // tokenise first word
    char* tokenPtr1 = strtok(buffer1, " ,\t\n");
    if(strcmp(tokenPtr,tokenPtr1)==0)
    {
        printf("MD5 matched\n");
    }
    else printf("MD5 no match\n");
    freeaddrinfo(my_info);
    return 0;
}
