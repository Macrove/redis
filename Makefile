all: hashtable.o utils.o client server

client: client.cpp utils.o
	g++ -std=c++17 client.cpp utils.o -o client

#hashtable.a: hashtable.o
#	ar rcs hashtable.a hashtable.o

hashtable.o: hashtable.cpp hashtable.h
	g++ -std=c++17 -c hashtable.cpp -o hashtable.o

utils.o: utils.cpp utils.h
	g++ -std=c++17 -c utils.cpp -o utils.o

server: server.cpp hashtable.o utils.o
	g++ -std=c++17 server.cpp hashtable.o utils.o -o server

clean:
	rm server client hashtable.o utils.o 

