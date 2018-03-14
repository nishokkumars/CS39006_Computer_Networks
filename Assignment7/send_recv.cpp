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

char senderBuffer[SENDER_BUFFER];
char receiverBuffer[RECV_BUFFER];
vector< pair< int, string > > recvBuffer;

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
	if(lastRecvd==expectedRecvd){

		int counter = expectedRecvd;
		vector< pair< int , string > >::iterator ptr;
		for(ptr=recvBuffer.begin();ptr<recvBuffer.end();ptr++)
		{
			 sort(recvBuffer.begin(),recvBuffer.end());
             recvBuffer.erase(unique(recvBuffer.begin(), recvBuffer.end()), recvBuffer.end());
			 if((*ptr).first == counter)
			 {
			 	 send_ack(sockfd,serveraddr,serverlen,counter);
			 	 counter++;
			 	 if(strlen(receiverBuffer)+(*ptr).second.length()>RECV_BUFFER)break;
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

int update_window(dataPacket d){
	/* updates window size on receiving the acks
	calls rate control */
	if(slowStart)
		cwnd = cwnd+MSS;
	else
		cwnd = cwnd+(MSS*MSS)/cwnd;
	return cwnd;
}


int parse_packets(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket d){
	/* Parses received packets, separating them as ack or data packets
	calls update window */
	if(strcpy(d.packetContents,"ACK")==0){
		update_window(d);
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

int rate_control(){
	/* handles flow and congestion control
	calls create packet */ 
	/* If timeout occurs make cwnd = 1mss and half the ssthresh 
	if triple ack then make ssthresh half and start from there */

}

int sendbuffer_handle(char* buffer){
	/* maintains queue of sender packets
	calls rate control */
	int i = strlen(senderBuffer);
	int k = 0;
	if(strlen(senderBuffer)>=SENDER_BUFFER)return -1;
	while(i<SENDER_BUFFER && k<strlen(buffer))
	{
		senderBuffer[i++] = buffer[k++];
	}
	senderBuffer[i] = '\0';
	return 0;
}


int appSend(){
	
	/* sends data directly to sender buffer 
	calls sendbuffer_handle */


}

int main(){
  
   return 0;
}