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
#include <bits/stdc++.h>
#include <math.h>

using namespace std;

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

typedef struct{
  user_info user;
  time_t last_activity;
}active;


user_info users[5];
active ac_users[5];

void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  users[0].name = (char *)"subham";
  users[0].ip = (char *)"10.145.170.183";
  users[0].port_no = 8085;

  users[1].name = (char *)"localhost";
  users[1].ip = (char *)"127.0.0.1";
  users[1].port_no = 8086;

  users[2].name = (char *)"server";
  users[2].ip = (char *)"10.5.18.69";
  users[2].port_no = 8086;

  users[3].name = (char *)"praj";
  users[3].ip = (char *)"10.145.194.229";
  users[3].port_no = 8085;

  users[4].name = (char *)"avinab";
  users[4].ip = (char *)"10.145.207.10";
  users[4].port_no = 8086;

  ac_users[0].user = users[0];
  ac_users[1].user = users[1];
  ac_users[2].user = users[2];
  ac_users[3].user = users[3];
  ac_users[4].user = users[4];



  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  unsigned int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr,serveraddr1; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  fileDetails f;
  fd_set master, read_fds;
  int i,j;
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
    error((char *)"ERROR opening socket");

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
    error((char *)"ERROR on binding");

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error((char *)"ERROR on listen");
  printf("Server Running ....\n");
  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(parentfd, &master);
  FD_SET(STDIN_FILENO,&master);

  int max_fd,fd;
  max_fd = parentfd;
  map<int, active> present;
  vector<int> activesocks;
  struct timeval t;
  t.tv_sec = 1;
  t.tv_usec =0;
  
  while(true){
    read_fds = master;
    if(select(max_fd+1,&read_fds, NULL, NULL, &t)>=0){
      for(i=0;i<=max_fd;i++){
        if(FD_ISSET(i,&read_fds)){
          if(i==parentfd){
            printf("Parent %d\n", fd);
            childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
            if (childfd < 0) 
              error((char *)"ERROR on accept");
            FD_SET(childfd, &master);
            max_fd = max(max_fd, childfd);
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , childfd , inet_ntoa(clientaddr.sin_addr) , ntohs(clientaddr.sin_port));
          }
          else if(i==STDIN_FILENO){
            bzero(buf,BUFSIZE);
            scanf(" %[^\n]s",buf);
            printf("%s\n", buf);
            char* hostname = strtok(buf,"/");
            char* msg = strtok(NULL,"\n");
            int sockfd;
            if (sockfd < 0) 
              error((char *)"ERROR opening socket");
            /* gethostbyname: get the server's DNS entry */
            char *recv_ip;
            int recv_port;
            for(j=0;j<5; j++){
              if(strcmp(users[j].name, hostname)==0){
                recv_ip = users[j].ip;
                recv_port = users[j].port_no;
                break;
              }
            }
            map<int, active>::iterator iter;
            int flag =0;
            active temp;
            for(iter = present.begin(); iter != present.end(); iter++){
              temp = iter->second;
              if(strcmp(temp.user.ip, recv_ip)==0 && temp.user.port_no == recv_port){
                sockfd = iter->first;
                flag = 1;
                break;
              }
            }
            if(flag == 1){
              n = send(sockfd, msg, BUFSIZE, 0);
              if(n<0)
                error((char *)"Error in send");
              time(&(present[sockfd].last_activity));
            }
            else{
              sockfd = socket(AF_INET, SOCK_STREAM, 0);
              if (sockfd < 0) 
                error((char *)"ERROR opening socket");
              struct hostent* server = gethostbyname(recv_ip);
              if (server == NULL) {
                fprintf(stderr,"ERROR, no such host as %s\n", hostname);
                exit(0);
              }
                 /* build the server's Internet address */
              bzero((char *) &serveraddr1, sizeof(serveraddr1));
              serveraddr1.sin_family = AF_INET;
              bcopy((char *)server->h_addr, (char *)&serveraddr1.sin_addr.s_addr, server->h_length);
              serveraddr1.sin_port = htons(recv_port);
              printf("%d\n", serveraddr1.sin_port);
                 //printf("%s\n", serveraddr1.sin_addr.s_addr );
              /* connect: create a connection with the server */
              if (connect(sockfd, (struct sockaddr*)&serveraddr1, sizeof(serveraddr1)) < 0){
                error((char *)"ERROR connecting");
              }
              activesocks.push_back(sockfd);
              active newtemp;
              newtemp.user.ip = recv_ip;
              newtemp.user.port_no = recv_port;
              time(&newtemp.last_activity);
              present[sockfd] = newtemp;
              FD_SET(sockfd, &master);
              max_fd = max(sockfd, max_fd);
              n = send(sockfd, msg, BUFSIZE, 0);
              if(n<0)
                error((char *)"Error in send");
            }
          }
          else{
            printf("Reading %d\n", fd);
            bzero(buf, BUFSIZE);
            n = recv(i,buf,BUFSIZE,0);
            if(n==0){
              close(i);
              FD_CLR(i, &master);
              continue;
            }
            if(n<0)
              error((char *)"Error in receive");
            buf[n] = '\0';
            //printf("Message :");
            //printf("%s\n",buf);          
            printf("%s messaged %s\n", users[i].name,buf);
          }
        }
      }
    }
    time_t curr;
    int len = activesocks.size();
    map<int, active>::iterator it;
    int k;
    for(k=0; k< len; k++){
      time(&curr);
      if(curr-present[activesocks[k]].last_activity>=20){
        printf("Closing connection %d\n", k);
        close(activesocks[k]);
        FD_CLR(activesocks[k],&master);
        it = present.find(activesocks[k]);
        present.erase(it);
        activesocks.erase(activesocks.begin()+k);
      }
    }
  }
  return 0;
}