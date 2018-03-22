#include "udpAPI.h"
int main(int argc,char* argv[]){
    /* sends data directly to sender buffer 
    calls sendbuffer_handle */
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct addrinfo *my_info,hints;
    struct hostent *server;
    char *hostname;
    //char *fileName;
    char buf[BUFSIZE];
    if(argc!=4)
    {
        cout<<"./a.out <hostname> <port-no> <filename>"<<endl;
        exit(0);
    }

    /* check command line arguments */
    hostname = (char *)argv[1];
    portno = atoi(argv[2]);
    struct timeval timeout = {1,0};
    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
    if (sockfd < 0) 
        perror("ERROR opening socket");

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

    // Check this 8080 thing 
    getaddrinfo(NULL, (char*)argv[2] , &hints, &my_info);

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    bzero(buf, BUFSIZE);
    serverlen = sizeof(serveraddr);
    /* get a message from the user */

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
    char buffer[MSS];
    bool endOfFile = feof(fp);
    int k,l;
    strcpy(buffer,argv[3]);
    appSend(sockfd,serveraddr,serverlen,buffer);
    memset(buffer,0,sizeof(buffer));
    string temp1 = to_string(fileSize);
    strcpy(buffer,temp1.c_str());
    appSend(sockfd,serveraddr,serverlen,buffer);    
    memset(buffer,0,sizeof(buffer));
    temp1 = to_string(noOfChunks);
    strcat(buffer,temp1.c_str());
    appSend(sockfd,serveraddr,serverlen,buffer);
    for(l=0;l<noOfChunks;l++){
        memset(buffer,0,sizeof(buffer));
        for(k=0;k<MSS && !endOfFile;k++){
            char nextChar = getc(fp);
            if(feof(fp)){
                endOfFile = true;
                k--;
            }
            else{
                buffer[k] = nextChar;
            }
        }
        buffer[k] = '\0';
        appSend(sockfd,serveraddr,serverlen,buffer);
    }
    char command[1024];
    strcpy(command,"md5sum ");
    strcat(command,argv[3]);
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
    cout<<buffer1<<endl;
    return 0;
}