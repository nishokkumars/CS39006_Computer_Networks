all: client server

client:	./Client/client.cpp
	g++ -std=c++11 -I. ./Client/client.cpp -lpthread -o ./Client/client

server:	./Server/server.cpp
	g++ -std=c++11 -I. ./Server/server.cpp -lpthread -o ./Server/server

clean:
	rm ./Client/client ./Server/server
