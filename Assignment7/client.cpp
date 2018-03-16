#include "udpAPI.h"
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
    recv_window_free = 9999;
    socket_details sock_det;
    udp_recv_str temp_recv;
    sock_det.sockfd = sockfd;
    sock_det.serveraddr = serveraddr;
    sock_det.serverlen = serverlen;
    temp_recv.sock = sock_det;
    appSend(filename);
    pthread_t udp_recv_thread, congestion_control_thread;
    pthread_create(&udp_recv_thread, NULL, udp_receive,&temp_recv );
    pthread_create(&congestion_control_thread, NULL, rate_control, &sock_det);
    pthread_join(congestion_control_thread,NULL);
    pthread_join(udp_recv_thread,NULL);
    pthread_mutex_destroy(&rlock);
    pthread_mutex_destroy(&slock);
    pthread_mutex_destroy(&other_var_lock);
    pthread_mutex_destroy(&send_buffer_lock);
    pthread_mutex_destroy(&recv_buffer_lock);
    return 0;
}