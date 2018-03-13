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

using namespace std;
int ssthresh;
int recv_window_free;
int slowStart;
int lastRecvd;
int expectedRecvd;
int cwnd;

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


vector< dataPacket > recvBuffer;

bool compare(const dataPacket& a,const dataPacket& b)
{
	return a.packetHeader.sequenceNumber < b.packetHeader.sequenceNumber;
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

int appRecv(dataPacket d){
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

int recvbuffer_handle(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
	/* handles receiver buffer */
	/* calls send_ack() */
	lastRecvd = d.packetHeader.sequenceNumber;
	if(lastRecvd==expectedRecvd){
		int counter = expectedRecvd;
		vector<dataPacket>::iterator ptr;
		for(ptr=recvBuffer.begin();ptr<recvBuffer.end();ptr++)
		{
			 sort(recvBuffer.begin(),recvBuffer.end(),compare);
             recvBuffer.erase(unique(recvBuffer.begin(), recvBuffer.end()), recvBuffer.end());
			 if(ptr->packetHeader.sequenceNumber == counter)
			 {
			 	 send_ack(sockfd,serveraddr,serverlen,counter);
			 	 counter++;
			 	 appRecv(*ptr);
			 	 recvBuffer.erase(recvBuffer.begin());
			 }
			 else break;
		} 
		
	}
	else if(lastRecvd > expectedRecvd){

        recvBuffer.push_back(d);
		send_ack(sockfd,serveraddr,serverlen, expectedRecvd);
        sort(recvBuffer.begin(),recvBuffer.end(),compare);
        recvBuffer.erase(unique(recvBuffer.begin(), recvBuffer.end()), recvBuffer.end());
		recv_window_free = recv_window_free-strlen(d.packetContents);
	}
}

int update_window(dataPacket d){
	/* updates window size on receiving the acks
	calls rate control */
	if(slowStart)
		cwnd = cwnd+MSS;
	else
		cwnd = cwnd+(MSS*MSS)/cwnd;

}


int parse_packets(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
	/* Parses received packets, separating them as ack or data packets
	calls update window */
	if(strcpy(d.packetContents,"ACK")==0){
		update_window(d);
	}
	else{
		recvbuffer_handle(sockfd, serveraddr, serverlen, d);
	}

}

int udp_receive(int sockfd,struct sockaddr_in clientaddr,int clientlen,dataPacket fileChunk){
	/* Sends udp packets - sending file details*/
    int n = recvfrom(sockfd, (char*)&fileChunk, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
    if(n<0)return 0;
    return parse_packets(sockfd, clientaddr, clientlen, fileChunk);
}

int rate_control(){
	/* handles flow and congestion control
	calls create packet */ 
	/* If timeout occurs make cwnd = 1mss and half the ssthresh 
	if triple ack then make ssthresh half and start from there */
}

int sendbuffer_handle(){
	/* maintains queue of sender packets
	calls rate control */
    /*while(nextseqnum<base+N){
         	
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
         }*/

}


int appSend(){
	/* sends data directly to sender buffer 
	calls sendbuffer_handle */
}









int main(){
  
   return 0;
}