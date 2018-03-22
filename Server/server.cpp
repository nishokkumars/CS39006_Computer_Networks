#include "udpAPI.h"
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
    clientlen = sizeof(clientaddr);
    char buff[RECV_BUFFER];
    while(1)
    {
        appRecv(sockfd,clientaddr,clientlen,buff);
        int fileSize;char fileName[50];int noOfChunks;
        strcpy(fileName,buff);
        appRecv(sockfd,clientaddr,clientlen,buff);
        fileSize = atoi(buff);
        appRecv(sockfd,clientaddr,clientlen,buff);
        noOfChunks = atoi(buff);
        cout<<fileName<<" "<<fileSize<<" "<<noOfChunks<<endl;
        int c1 = 0;
        FILE* fp = fopen(fileName,"wb");
        while(c1<noOfChunks)
        {
            appRecv(sockfd,clientaddr,clientlen,buff);
            unsigned int i;
            for(i = 0; i < strlen(buff); ++i) {
                fputc(buff[i],fp);
            }
            c1++;

        }
        fclose(fp);
        char command[1024];
        strcpy(command,"md5sum ");
        strcat(command,fileName);
        FILE *md5_cmd = popen(command, "r");
        if (md5_cmd == NULL) {
          fprintf(stderr, "popen(3) error");
          exit(EXIT_FAILURE);
        }
        static char buffer[1024];
        size_t n;
        memset(buffer,0,1024);
        while ((n = fread(buffer, 1, sizeof(buffer)-1, md5_cmd)) > 0) {
           
           buffer[n] = '\0';
           break;
        }
        cout<<buffer<<endl;
        /*appSend(sockfd,clientaddr,clientlen,buffer);*/
        exit(0);
    }
    return 0;
}
