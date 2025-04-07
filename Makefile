all: client server

client: client.cpp
	g++ -std=c++17 client.cpp -o client

server: server.cpp
	g++ -std=c++17 server.cpp -o server

clean:
	rm server client

