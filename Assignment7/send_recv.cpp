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


int udp_send(){
	/* Sends udp packets */
}

int create_packet(){
	/* creates packets 
	calls udp_send */
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

int update_window(){
	/* updates window size on receiving the acks
	calls rate control */
}

int parse_packets(){
	/* Parses received packets, separating them as ack or data packets
	calls update window */
}

int udp_receive(){
	/* Receives udp packets
	calls parse packets */ 
}


int send_ack(){
	/* construct ack */
	/* calls udp_send */
}

int recvbuffer_handle(){
	/* handles receiver buffer */
	/* calls send_ack() */
}

int appSend(){
	/* sends data directly to sender buffer 
	calls sendbuffer_handle */
}

int appRecv(){
	/* calls recvbuffer_handle and gets data from there
	is blocked if no data in buffer */
}






int main(){

}