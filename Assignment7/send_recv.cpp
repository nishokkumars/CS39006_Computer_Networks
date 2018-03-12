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
#define BUFSIZE 1024 // to divide into chunks of 1 KB

struct fileDetails {
   
   char fileName[512];
   int fileSize;
   int noOfDataChunks;

};

struct header{
  
   int sequenceNumber;
   int chunkLength;

};

struct dataPacket{
 
   header packetHeader;
   char packetContents[1024];

};

int udp_send(int sockfd,struct sockaddr_in serveraddr,int serverlen,dataPacket fileChunk){
	/* Sends udp packets - sending file details*/
    int n = sendto(sockfd, (char*)(&fileChunk), sizeof(fileChunk), 0, ((sockaddr*)&serveraddr), serverlen);
    if(n<0)return 0;
    return 1;
}

int udp_send(int sockfd,struct sockaddr_in serveraddr,int serverlen,fileDetails fileChunk){
	/* Sends udp packets - sending file data in chunks*/
    int n = sendto(sockfd, (char*)(&fileChunk), sizeof(fileChunk), 0, ((sockaddr*)&serveraddr), serverlen);
    if(n<0)return 0;
    return 1;
}

int udp_send(int sockfd,struct sockaddr_in serveraddr,int serverlen,int ack){
	/* Sends udp packets - for ack */
    int n = sendto(sockfd, (char*)(&ack), sizeof(ack), 0, ((sockaddr*)&serveraddr), serverlen);
    if(n<0)return 0;
    return 1;
}

int create_file_details_packet(int sockfd,struct sockaddr_in serveraddr,int serverlen,char fileName[])
{

    struct stat inputFileInfo;
    // Get filesize to calculate total number of fragments and total number of fragment digits
    stat(fileName, &inputFileInfo);
    int fileSize = inputFileInfo.st_size;
    int noOfChunks = fileSize%1024 == 0 ? fileSize/1024 : (fileSize/1024+1);
    fileDetails f;
    strcpy(f.fileName,fileName);
    f.fileSize = fileSize;
    f.noOfDataChunks = noOfChunks;
    return udp_send(sockfd,serveraddr,serverlen,f);
}

int create_data_packet(int sockfd,struct sockaddr_in serveraddr,int serverlen,char* data,int seqNo){
	/* creates packets 
	calls udp_send */
	dataPacket d;
	strcpy(d.packetContents,data);
	d.packetHeader.sequenceNumber = seqNo;
	d.packetHeader.chunkLength = strlen(d.packetContents);
	return udp_send(sockfd,serveraddr,serverlen,d);

}

int udp_receive(int sockfd,struct sockaddr_in clientaddr,int clientlen,dataPacket fileChunk){
	/* Sends udp packets - sending file details*/
    int n = recvfrom(sockfd, (char*)&fileChunk, sizeof(fileChunk), 0,(struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
    if(n<0)return 0;
    return 1;
}

int udp_receive(int sockfd,struct sockaddr_in clientaddr,int clientlen,fileDetails fileChunk){
	/* Sends udp packets - sending file data in chunks*/
    int n = recvfrom(sockfd, (char*)(&fileChunk), sizeof(fileChunk), 0, ((sockaddr*)&clientaddr), (socklen_t*)&clientlen);
    if(n<0)return 0;
    return 1;
}

int udp_receive(int sockfd,struct sockaddr_in clientaddr,int clientlen,int ack){
	/* Sends udp packets - for ack */
    int n = recvfrom(sockfd, (char*)(&ack), sizeof(ack), 0, ((sockaddr*)&clientaddr), (socklen_t*)&clientlen);
    if(n<0)return 0;
    return 1;
}

int parse_packets(){
	/* Parses received packets, separating them as ack or data packets
	calls update window */
}

int update_window(){
	/* updates window size on receiving the acks
	calls rate control */
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

}


int appSend(){
	/* sends data directly to sender buffer 
	calls sendbuffer_handle */
}

int send_ack(){
	/* construct ack */
	/* calls udp_send */
}

int recvbuffer_handle(){
	/* handles receiver buffer */
	/* calls send_ack() */
}


int appRecv(){
	/* calls recvbuffer_handle and gets data from there
	is blocked if no data in buffer */
}






int main(){
  
   return 0;
}