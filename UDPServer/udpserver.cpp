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
#include <time.h>
#include <bits/stdc++.h>
#include <fcntl.h>

#define MSS 1024 // to divide into chunks of 1 KB
#define SENDER_BUFFER 1024*1024 // Buffer size
#define RECV_BUFFER 1024*1024 // Receiver buffer size
#define TIMEOUT_VAL 1 
#define BUFSIZE 1024

using namespace std;
int ssthresh;
int recv_window_free;
int slowStart;
int lastRecvd;
int expectedRecvd;
int cwnd;
int base;
int nextseqnum;
int tempp;
int ack;
int prevReceived;
time_t startTime,endTime;
map< int, int > dupAckCount;

struct header{
  
   int sequenceNumber;
   int chunkLength;
   int recv_window_left;
   bool operator==(header& rhs)const{

   	  return sequenceNumber == rhs.sequenceNumber && chunkLength == rhs.chunkLength && recv_window_left == rhs.recv_window_left;
   }

};

struct dataPacket{
 
   header packetHeader;
   char packetContents[1024];
   bool operator==(dataPacket& rhs)const{

         return packetHeader == rhs.packetHeader &&  ( strcmp(packetContents,rhs.packetContents) == 0 ) ;
   }

};

char senderBuffer[SENDER_BUFFER];
char receiverBuffer[RECV_BUFFER];
vector< pair< int, string > > recvBuffer;
 
void error(char *msg) {
  perror(msg);
  exit(1);
}
int udp_send(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket fileChunk){
	/* Sends udp packets - sending file details */
    int n = sendto(sockfd, (char*)(&fileChunk), sizeof(fileChunk), 0, ((sockaddr*)&serveraddr), serverlen);
    if(n<0)return 0;
    return 1;
}


int create_data_packet(int sockfd,struct sockaddr_in serveraddr,int serverlen,char* data,int seqNo){
	/* creates packets 
	calls udp_send */
	dataPacket d;
	strcpy(d.packetContents,data);
	d.packetHeader.sequenceNumber = seqNo;
	d.packetHeader.chunkLength = strlen(d.packetContents);
	if(strcmp(data,"ACK")==0)
		d.packetHeader.recv_window_left = recv_window_free;
	else
		d.packetHeader.recv_window_left = -9999;
  cout<<"sent ack "<<d.packetHeader.sequenceNumber<<endl;
	return udp_send(sockfd,serveraddr,serverlen,d);

}

int appRecv(){
	/* calls recvbuffer_handle and gets data from there
	is blocked if no data in buffer */
	int fd = creat("ReceivedData.txt",0666);


}

int send_ack(int sockfd,struct sockaddr_in serveraddr,int serverlen, int seqNo){
	/* construct ack */
	/* calls udp_send */
	char *ack = (char *)"ACK";
	create_data_packet(sockfd, serveraddr, serverlen,ack, seqNo);

}

int recvbuffer_handle(int sockfd,struct sockaddr_in serveraddr,int serverlen,char* buffer,int lastRecvd){
	/* handles receiver buffer */
	/* calls send_ack() */
	if(lastRecvd==expectedRecvd){
        
		int counter = expectedRecvd;
		vector< pair< int , string > >::iterator ptr;
		bool flag= false;
		recvBuffer.push_back(make_pair(lastRecvd,buffer));
		for(ptr=recvBuffer.begin();ptr<recvBuffer.end();ptr++)
		{
			 sort(recvBuffer.begin(),recvBuffer.end());
             recvBuffer.erase(unique(recvBuffer.begin(), recvBuffer.end()), recvBuffer.end());
			 if((*ptr).first == counter)
			 {
			 	 send_ack(sockfd,serveraddr,serverlen,counter);
			 	 counter++;
			 	 if(strlen(receiverBuffer)+(*ptr).second.length()>RECV_BUFFER && flag == false){flag=1;continue;}
			 	 char temp[MSS+5];
			 	 strcpy(temp,(*ptr).second.c_str());
			 	 strcat(receiverBuffer,temp);
			 	 recvBuffer.erase(recvBuffer.begin());
			 }
			 else break;
		} 
		expectedRecvd = counter;
	
	}
	else if(lastRecvd > expectedRecvd){

        recvBuffer.push_back(make_pair(lastRecvd,string(buffer)));
		send_ack(sockfd,serveraddr,serverlen,expectedRecvd);
        sort(recvBuffer.begin(),recvBuffer.end());
        recvBuffer.erase(unique(recvBuffer.begin(), recvBuffer.end()), recvBuffer.end());
		recv_window_free = recv_window_free-strlen(buffer);
	
	}
	return 0;
}

int update_window(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
	/* updates window size on receiving the acks
	calls rate control */
	if(slowStart)
		cwnd = cwnd+MSS;
	else
		cwnd = cwnd+(MSS*MSS)/cwnd;
	//get lock
    ack = d.packetHeader.sequenceNumber;
	//unlock
	//rate_control(sockfd,serveraddr, serverlen);
	return cwnd;
}


int parse_packets(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
	/* Parses received packets, separating them as ack or data packets
	calls update window */
	if(strcmp(d.packetContents,"ACK")==0){
		update_window(sockfd,serveraddr, serverlen,d);
	}
	else{

		recvbuffer_handle(sockfd, serveraddr, serverlen,d.packetContents,d.packetHeader.sequenceNumber);
	}

}

int udp_receive(int sockfd,struct sockaddr_in clientaddr,int clientlen,dataPacket fileChunk){
	/* Sends udp packets - sending file details*/
    int n = recvfrom(sockfd, (char*)&fileChunk, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
    if(n<0)return 0;
    return parse_packets(sockfd, clientaddr, clientlen, fileChunk);
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
    receiverBuffer[0] = '\0';
	cwnd = 1;
	base = 1;
	expectedRecvd = 1;
	nextseqnum = 1;
	lastRecvd = 0;
	ssthresh = 340*1024;
  recv_window_free = 9999;
    dataPacket p;
    //printf("%d %d",noOfReceivedPackets,f.noOfChunks);
    FILE* fp = fopen("test.cpp","wb");
    int expectedseqnum = 1;
    int prevAck = 0;
    while(1)
    {
         int n = recvfrom(sockfd, (char*)&p, sizeof(p), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
         cout<<"hi"<<" "<<p.packetContents<<endl;
         int seqNo = p.packetHeader.sequenceNumber;
         printf("%d\n",seqNo);
         int proba = rand()%1000;
         if(proba<=(int)drop_probability*1000)
         send_ack(sockfd,clientaddr,clientlen,seqNo);
         expectedseqnum++;
     }
     fclose(fp);
     cout<<receiverBuffer<<endl;
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
    n = sendto(sockfd, buffer, strlen(buffer), 0,(struct sockaddr *) &clientaddr, (socklen_t)clientlen);
    if (n < 0) 
        error("ERROR in sendto");

  }
  return 0;
}
