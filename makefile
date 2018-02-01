all: udpserver udpclient tcpserver tcpclient
	 rm ./UDPClient/*.o ./UDPServer/*.o ./TCPClient/*.o ./TCPServer/*.o
	 
udpserver: udpserver.o
	gcc -g ./UDPServer/udpserver.o -o ./UDPServer/udpserver -lm

udpclient: udpclient.o
	gcc -g ./UDPClient/udpclient.o -o ./UDPClient/udpclient  -lm

udpclient.o: ./UDPClient/udpclient.c 
	gcc -g ./UDPClient/udpclient.c -c -o ./UDPClient/udpclient.o

udpserver.o: ./UDPServer/udpserver.c
	gcc -g ./UDPServer/udpserver.c -c -o ./UDPServer/udpserver.o

tcpserver: tcpserver.o
	gcc -g ./TCPServer/tcpserver.o -o ./TCPServer/tcpserver -lm

tcpclient: tcpclient.o
	gcc -g ./TCPClient/tcpclient.o -o ./TCPClient/tcpclient -lm

tcpclient.o: ./TCPClient/tcpclient2.c 
	gcc -g ./TCPClient/tcpclient2.c -c -o ./TCPClient/tcpclient.o

tcpserver.o: ./TCPServer/tcpserver2.c
	gcc -g ./TCPServer/tcpserver2.c -c -o ./TCPServer/tcpserver.o

clean:
	rm ./UDPClient/udpclient ./UDPServer/udpserver ./TCPServer/tcpserver ./TCPClient/tcpclient
