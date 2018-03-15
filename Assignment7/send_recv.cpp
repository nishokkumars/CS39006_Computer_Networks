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
#include <pthread.h>

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
int prevReceived;
int ack;
time_t startTime,endTime;
map< int, int > dupAckCount;

pthread_mutex_t slock;// Sender lock
pthread_mutex_t rlock; // Receiver lock
pthread_mutex_t send_buffer_lock; 
pthread_mutex_t recv_buffer_lock;
pthread_mutex_t other_var_lock;


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

typedef struct{
    int sockfd;
    struct sockaddr_in serveraddr;
    int serverlen;
}socket_details;

typedef struct{
    socket_details sock;
    dataPacket d;
}udp_recv_str;

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
  pthread_mutex_lock(&recv_buffer_lock);
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
  pthread_mutex_unlock(&recv_buffer_lock);
	return 0;
}

void * rate_control(void * param){
	/* handles flow and congestion control
	calls create packet */ 
	/* If timeout occurs make cwnd = 1mss and half the ssthresh 
	if triple ack then make ssthresh half and start from there */
    socket_details * sock = (socket_details *)param;
    int sockfd = sock->sockfd;
    struct sockaddr_in serveraddr = sock->serveraddr;
    int serverlen = sock->serverlen;
    /* 
    Used lock here
     */
    pthread_mutex_lock(&other_var_lock);
    int N = min(cwnd,max(recv_window_free,0));
    int n;
    int temp2 = tempp;
    pthread_mutex_unlock(&other_var_lock);
    while(base<=int((ceil(1.0*strlen(senderBuffer)/MSS))))
    {
      while(nextseqnum<base+N){
        char *to_be_sent_data = (char *)malloc(MSS*sizeof(char));
        int lastSeqSent = nextseqnum-1;
        int noOfBytesSent = lastSeqSent*MSS;
        int temp1 = min(MSS,max((int)strlen(senderBuffer)-noOfBytesSent,0));
        strncpy(to_be_sent_data,senderBuffer+tempp,temp1);
        n = create_data_packet(sockfd, serveraddr, serverlen,to_be_sent_data,nextseqnum);
        if(base == nextseqnum){
          //start timer
          time(&startTime);
        }
        nextseqnum++;
        tempp+=temp1;
      }
      int flag = 0;
      int tripleDupAck,timeout;
    /*while(1)
    {
        time(&endTime);
        if(endTime-startTime>=N*TIMEOUT_VAL)
        {
        	flag = 1;
        	break;ment 4 
        }
        else
        	break;

    }*/
    if(flag==1){
    	int ptr;
    	timeout = 1;
        char *to_be_sent_data = (char *)malloc(MSS*sizeof(char));
        for(ptr = base; ptr < nextseqnum; ptr++){
        	bzero(to_be_sent_data,MSS);
	         int lastSeqSent = ptr-1;
	         int noOfBytesSent = lastSeqSent*MSS;
	         int temp1 = min(MSS,(int)strlen(senderBuffer)-noOfBytesSent);
	         strncpy(to_be_sent_data,senderBuffer+temp2,temp1);
        	n = create_data_packet(sockfd, serveraddr, serverlen,to_be_sent_data,ptr);
        	temp2+=temp1;
        }


    }
    //receive signal from window_update
    //update window
    if(prevReceived == ack)dupAckCount[prevReceived]++;
    if(dupAckCount[prevReceived] == 3)tripleDupAck = 1;
    prevReceived = ack;
    if( base == nextseqnum )
    {      
        ;   
    
    }else{
         
      if(timeout){
        /* used lock here */
        pthread_mutex_lock(&other_var_lock);
  		  ssthresh = ssthresh/2;
  		  cwnd = MSS;
  		  slowStart = 1;
        pthread_mutex_unlock(&other_var_lock);
	   }
	    else if(tripleDupAck)
	   {
      /* Used lock here */
       pthread_mutex_lock(&other_var_lock);
  		 ssthresh/=2;
  		 cwnd = ssthresh;
  		 slowStart = 0;
       pthread_mutex_unlock(&other_var_lock);
	   }
    }
  }

}

int update_window(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
	/* updates window size on receiving the acks
	calls rate control */
  /*
  Used lock here 
  */
	if(slowStart){
    pthread_mutex_lock(&other_var_lock);
		cwnd = cwnd+MSS;
    pthread_mutex_unlock(&other_var_lock);
  }
	else{
    pthread_mutex_lock(&other_var_lock);
		cwnd = cwnd+(MSS*MSS)/cwnd;
    pthread_mutex_lock(&other_var_lock);
  }
	//get lock
  pthread_mutex_lock(&other_var_lock);
  ack = d.packetHeader.sequenceNumber;
  recv_window_free = d.packetHeader.recv_window_left;
  pthread_mutex_unlock(&other_var_lock);
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

void * udp_receive(void *param){
	/* Sends udp packets - sending file details*/
  /* 
  Not sure of the locks here 
  */
    udp_recv_str * recvd_details = (udp_recv_str *)param;
    int sockfd = recvd_details->sock.sockfd;
    struct sockaddr_in clientaddr = recvd_details->sock.serveraddr;
    int clientlen = recvd_details->sock.serverlen;
    dataPacket fileChunk = recvd_details->d;
    int n = recvfrom(sockfd, (char*)&fileChunk, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
    if(n<0)return 0;
    parse_packets(sockfd, clientaddr, clientlen, fileChunk);
}



int sendbuffer_handle(char* buffer){
	/* maintains queue of sender packets
	calls rate control */
	int i = strlen(senderBuffer);
	int k = 0;
	if(strlen(senderBuffer)>=SENDER_BUFFER)return 0;

    /* 
    Used lock Here
     */
    pthread_mutex_lock(&send_buffer_lock);
	while(i<SENDER_BUFFER && k<strlen(buffer))
	{
		senderBuffer[i++] = buffer[k++];
	}
	senderBuffer[i] = '\0';
    pthread_mutex_unlock(&send_buffer_lock);
	return k;
}


int appSend(socket_details sock_det, char * buffer2){
	/*
     How to use lock here I am not sure
      so please look into this 
      */
	
	 FILE *fp = fopen(buffer2,"rb");
	 if (fp == NULL) {
       printf("File does not exist \n");
       return 1;
    }
    struct stat inputFileInfo;
    // Get filesize to calculate total number of fragments and total number of fragment digits
    stat(buffer2, &inputFileInfo);
    int fileSize = inputFileInfo.st_size;
    int noOfChunks = fileSize%1024 == 0 ? fileSize/1024 : (fileSize/1024+1);
    char *buffer= (char *)malloc((MSS+1)*sizeof(char));
    bool endOfFile = feof(fp);
    int k,l;
    for(l=0;l<noOfChunks;l++){
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
    	buffer[k]='\0';
    	sendbuffer_handle(buffer);
    	//rate_control((void *)sock_det);
    }

}

int main(){
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

    /* check command line arguments */
    hostname = (char *)"127.0.0.1";
    portno = 8080;
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
    getaddrinfo(NULL, (char*)"8080" , &hints, &my_info);

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    bzero(buf, BUFSIZE);
    serverlen = sizeof(serveraddr);
    /* get a message from the user */

    if (pthread_mutex_init(&rlock, NULL) != 0)
    {
        printf("\n window mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&slock, NULL) != 0)
    {
        printf("\n sender mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&other_var_lock, NULL) != 0)
    {
        printf("\n receiver  mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&send_buffer_lock, NULL) != 0)
    {
        printf("\n receiver  mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&recv_buffer_lock, NULL) != 0)
    {
        printf("\n receiver  mutex init has failed\n");
        return 1;
    }

	char *filename = (char *)malloc(1000*sizeof(char));
	cout << "Enter filename" << endl;
	cin >> filename ;
	cwnd = 1;
	base = 1;
	expectedRecvd = 1;
	nextseqnum = 1;
	lastRecvd = 0;
	ssthresh = 340*1024;
    socket_details sock_det;
    udp_recv_str temp_recv;
    sock_det.sockfd = sockfd;
    sock_det.serveraddr = serveraddr;
    sock_det.serverlen = serverlen;
    temp_recv.sock = sock_det;
    pthread_t udp_recv_thread, congestion_control_thread;
    pthread_create(&udp_recv_thread, NULL, udp_receive,&temp_recv );
    pthread_create(&congestion_control_thread, NULL, rate_control, &sock_det);
  	appSend(sock_det, filename);
    pthread_mutex_destroy(&rlock);
    pthread_mutex_destroy(&slock);
    pthread_mutex_destroy(&other_var_lock);
    pthread_mutex_destroy(&send_buffer_lock);
    pthread_mutex_destroy(&recv_buffer_lock);
    return 0;
  
   return 0;
}