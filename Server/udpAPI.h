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
bool ackArrived;
int senderBufferLength;
int receiverBufferLength;
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
bool myfunction (pair< int, dataPacket > lhs,pair< int, dataPacket> rhs) { return (lhs.first < rhs.first ); }

struct socket_details{
    
    int sockfd;
    sockaddr_in serveraddr;
    int serverlen;

};

struct udp_recv_str{
    
    socket_details sock;
    dataPacket d;

};

char senderBuffer[SENDER_BUFFER];
char receiverBuffer[RECV_BUFFER];
vector< pair< int, dataPacket > > recvBuffer;
 
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


int create_data_packet(int sockfd,struct sockaddr_in serveraddr,int serverlen,char* data,int len,int seqNo){
  /* creates packets 
  calls udp_send */
  dataPacket d;
  int it;

  for(it=0;it<len;it++)
    d.packetContents[it] = data[it];
  d.packetContents[it] = '\0';

  d.packetHeader.sequenceNumber = seqNo;
  d.packetHeader.chunkLength = len;
  if(strcmp(data,"ACK")==0)
    d.packetHeader.recv_window_left = recv_window_free;
  else
    d.packetHeader.recv_window_left = -9999;
  return udp_send(sockfd,serveraddr,serverlen,d);

}

int send_ack(int sockfd,struct sockaddr_in serveraddr,int serverlen, int seqNo){
  /* construct ack */
  /* calls udp_send */
  char *ack = (char *)"ACK";
  create_data_packet(sockfd, serveraddr, serverlen,ack,3,seqNo);

}

int recvbuffer_handle(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
  /* handles receiver buffer */
  /* calls send_ack() */
  lastRecvd = d.packetHeader.sequenceNumber;
  if(strcmp(d.packetContents,"finish")==0)
  {
    send_ack(sockfd,serveraddr,serverlen,-1);
    return -1;
  }
  if(lastRecvd==expectedRecvd){
        
    int counter = expectedRecvd;
    vector< pair< int , dataPacket > >::iterator ptr;
    bool flag= false;
    recvBuffer.push_back(make_pair(lastRecvd,d));
    sort(recvBuffer.begin(),recvBuffer.end(),myfunction);
    for(ptr=recvBuffer.begin();ptr<recvBuffer.end();ptr++)
    {

       if((*ptr).first == counter)
       {
         send_ack(sockfd,serveraddr,serverlen,counter);
         counter++;
         expectedRecvd++;
         char temp[MSS];
         int it;
         for(it=0;it<(*ptr).second.packetHeader.chunkLength;it++)
          temp[it] = (*ptr).second.packetContents[it];
          temp[it] = '\0';
         for(int it1=0;it1<it;it1++)
         receiverBuffer[receiverBufferLength++] = temp[it1];
         recvBuffer.erase(recvBuffer.begin());
       }
       else if((*ptr).first>counter)break;
    } 
  
  }
  else if(lastRecvd > expectedRecvd){

        recvBuffer.push_back(make_pair(lastRecvd,d));
    send_ack(sockfd,serveraddr,serverlen,expectedRecvd);
        sort(recvBuffer.begin(),recvBuffer.end(),myfunction);
    recv_window_free = recv_window_free-d.packetHeader.chunkLength;
  
  }
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
    int tt;
    pthread_mutex_lock(&other_var_lock);
    if((senderBufferLength)%MSS == 0)
    {
       tt = senderBufferLength/MSS;
    }else tt = senderBufferLength/MSS +1;
    pthread_mutex_unlock(&other_var_lock);
    int temp2;
    while(base<=tt)
    {
      pthread_mutex_lock(&other_var_lock);
      int N = min(cwnd,max(recv_window_free,0));
      int n;
      pthread_mutex_unlock(&other_var_lock);
      temp2 = tempp;
       while(nextseqnum<base+N && nextseqnum <=tt){
        
        char to_be_sent_data[MSS]={0};
        int lastSeqSent = nextseqnum-1;
        int noOfBytesSent = lastSeqSent*MSS;
        pthread_mutex_lock(&send_buffer_lock);
        int temp1 = min(MSS,max(senderBufferLength-noOfBytesSent,0));
        int it;
        for(it=tempp;it<tempp+temp1;it++)
          to_be_sent_data[it-tempp] = senderBuffer[it];
        to_be_sent_data[it-tempp] = '\0';
        pthread_mutex_unlock(&send_buffer_lock);
        n = create_data_packet(sockfd, serveraddr, serverlen,to_be_sent_data,temp1,nextseqnum);
        if(base == nextseqnum){
          //start timer
          time(&startTime);
        }
        nextseqnum++;
        tempp+=temp1;
      }
      int flag = 0;
      int tripleDupAck,timeout;
      int temp3=0;;
    while(1)
    {
        pthread_mutex_lock(&other_var_lock);
        if(ackArrived == true){temp3 = 1;ackArrived = false;}
        pthread_mutex_unlock(&other_var_lock);
        if(temp3==1)break;
        time(&endTime);
        if(endTime-startTime>=N*TIMEOUT_VAL)
        {
          flag = 1;
          break; 
        }

    }
    if(flag==1){
      int ptr;
      timeout = 1;
        ssthresh = ssthresh/2;
        cwnd = 1;
        slowStart = 1;
        tempp = temp2;
        nextseqnum = base;
        continue;

    }
    //receive signal from window_update
    //update window
    pthread_mutex_lock(&other_var_lock);
    if(ack==-1){
      pthread_mutex_unlock(&other_var_lock);
            timeout = 1;
        ssthresh = ssthresh/2;
        cwnd = 1;
        slowStart = 1;
        tempp = temp2;
        nextseqnum = base;
        continue;
      }
    base = ack+1;
    if(prevReceived == ack)dupAckCount[prevReceived]++;
    if(dupAckCount[prevReceived] == 3)tripleDupAck = 1;
    prevReceived = ack;
    if(cwnd>=ssthresh)
    {
      slowStart = 0;
    }
    if( base == nextseqnum )
    {      
        ;   
    
    }else{
         
      if(timeout){
        /* used lock here */
        ssthresh = ssthresh/2;
        cwnd = 1;
        slowStart = 1;
     }
      else if(tripleDupAck)
     {
      /* Used lock here */
       ssthresh/=2;
       cwnd = ssthresh/MSS;
       slowStart = 0;
       
     }
    }
    pthread_mutex_unlock(&other_var_lock);
  }
  int p = 0;
  while(1)
  {
     int n = create_data_packet(sockfd, serveraddr, serverlen,(char*)"finish",6,-1);
     time(&startTime);
     int flag = 0;
     int temp3 = 0;
     while(1)
     {
        pthread_mutex_lock(&other_var_lock);
        if(ackArrived == true){temp3 = 1;ackArrived = false;}
        pthread_mutex_unlock(&other_var_lock);
        if(temp3==1)break;
        time(&endTime);
        if(endTime-startTime>=TIMEOUT_VAL)
        {
          flag = 1;
          p++;
          break; 
        }

     }
     if(p==3)break;
     pthread_mutex_lock(&other_var_lock);
     base = ack+1;
     if(ack == -1){pthread_mutex_unlock(&other_var_lock);break;}
     pthread_mutex_unlock(&other_var_lock);
  }
}

int update_window(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
  /* updates window size on receiving the acks
  calls rate control */
  /*
  Used lock here 
  */
  pthread_mutex_lock(&other_var_lock);
  if(slowStart){
    cwnd = 2*cwnd;
  }
  else{
    
    cwnd = cwnd+(MSS)/cwnd;
  
  }
  //get lock
  ack = d.packetHeader.sequenceNumber;
  ackArrived = 1;
  recv_window_free = d.packetHeader.recv_window_left;
  int temp = cwnd;
  pthread_mutex_unlock(&other_var_lock);
  //unlock
  //rate_control(sockfd,serveraddr, serverlen);
  return temp;
}


int parse_packets(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
  /* Parses received packets, separating them as ack or data packets
  calls update window */
  if(strcmp(d.packetContents,"ACK")==0){
    update_window(sockfd,serveraddr, serverlen,d);
    if(d.packetHeader.sequenceNumber == -1){return -1;}

  }
  else{

    return recvbuffer_handle(sockfd, serveraddr, serverlen,d);
  }
  return 0;

}

void * udp_receive(void *param){
  /* Sends udp packets - sending file details*/

    udp_recv_str * recvd_details = (udp_recv_str *)param;
    int sockfd = recvd_details->sock.sockfd;
    struct sockaddr_in clientaddr = recvd_details->sock.serveraddr;
    int clientlen = recvd_details->sock.serverlen;
    dataPacket fileChunk = recvd_details->d;
    while(1)
    {
      int n = recvfrom(sockfd, (char*)&fileChunk, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
      int temp;
      if(n>=0)
       temp = parse_packets(sockfd, clientaddr, clientlen, fileChunk);
      if(temp <0)break;
    }
}

int udp_receive(int sockfd,struct sockaddr_in& clientaddr,int clientlen,dataPacket fileChunk){
  /* Sends udp packets - sending file details*/
    int n = recvfrom(sockfd, (char*)&fileChunk, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
    if(n<0)return 0;
    return parse_packets(sockfd, clientaddr, clientlen, fileChunk);
}
void clearRecvBuffer()
{
  memset(receiverBuffer,0,RECV_BUFFER);

}

int appRecv(int sockfd,struct sockaddr_in& clientaddr,int clientlen,char buffer[RECV_BUFFER]){
  /* calls recvbuffer_handle and gets data from there
  is blocked if no data in buffer */
    cwnd = 3;
    base = 1;
    expectedRecvd = 1;
    nextseqnum = 1;
    lastRecvd = 0;
    ssthresh = 340*1024;
    recv_window_free = RECV_BUFFER;
    receiverBufferLength = 0;
    clearRecvBuffer();
    dataPacket p;
    while(1)
    {    
         int n = udp_receive(sockfd,clientaddr,clientlen,p);
         if(n<0)break;
     }
    int it;
    for(it=0;it<receiverBufferLength;it++)
      buffer[it] = receiverBuffer[it];
     buffer[it] = '\0';
     clearRecvBuffer();
     return receiverBufferLength;
}

int sendbuffer_handle(char* buffer,int length){
  /* maintains queue of sender packets
  calls rate control */
  pthread_mutex_lock(&send_buffer_lock);
  int i = 0;
  int k = 0;
  /* 
    Used lock Here
  */
  while(i<SENDER_BUFFER && k<length)
  {
    senderBuffer[i++] = buffer[k++];
  }
  senderBufferLength = length;
  pthread_mutex_unlock(&send_buffer_lock);
  return k;

}
void clearSendBuffer()
{
   pthread_mutex_lock(&send_buffer_lock);
   memset(senderBuffer,0,SENDER_BUFFER);
   pthread_mutex_unlock(&send_buffer_lock);
}

int init()
{
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
}
void destroy()
{
    pthread_mutex_destroy(&rlock);
    pthread_mutex_destroy(&slock);
    pthread_mutex_destroy(&other_var_lock);
    pthread_mutex_destroy(&send_buffer_lock);
    pthread_mutex_destroy(&recv_buffer_lock);
}
int appSend(int sockfd,struct sockaddr_in serveraddr,int serverlen,char * buffer2,int length){

    socket_details sock_det;
    udp_recv_str temp_recv;
    sock_det.sockfd = sockfd;
    sock_det.serveraddr = serveraddr;
    sock_det.serverlen = serverlen;
    temp_recv.sock = sock_det;
    cwnd = 3;
    base = 1;
    expectedRecvd = 1;
    nextseqnum = 1;
    lastRecvd = 0;
    ssthresh = 340*1024;
    recv_window_free = RECV_BUFFER;
    tempp = 0;
    slowStart = 1;
    prevReceived = -1;
    senderBufferLength = 0;
    init();
    clearSendBuffer();
    sendbuffer_handle(buffer2,length);
    pthread_t udp_recv_thread, congestion_control_thread;
    pthread_create(&udp_recv_thread, NULL, udp_receive,&temp_recv );
    pthread_create(&congestion_control_thread, NULL, rate_control, &sock_det);
    pthread_join(congestion_control_thread,NULL);
    pthread_join(udp_recv_thread,NULL);
    clearSendBuffer();
    destroy();
}