all: udpserver udpclient
	 rm ./UDPClient/*.o ./UDPServer/*.o
	 
udpserver: udpserver.o
	gcc -g ./UDPServer/udpserver.o -o ./UDPServer/udpserver -lm

udpclient: udpclient.o
	gcc -g ./UDPClient/udpclient.o -o ./UDPClient/udpclient  -lm

udpclient.o: ./UDPClient/udpclient.c 
	gcc -g ./UDPClient/udpclient.c -c -o ./UDPClient/udpclient.o

udpserver.o: ./UDPServer/udpserver.c
	gcc -g ./UDPServer/udpserver.c -c -o ./UDPServer/udpserver.o

clean:
	rm ./UDPClient/udpclient ./UDPServer/udpserver
