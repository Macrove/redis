CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra
DEBUG_FLAGS = -g -O0

all: server client

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

utils.o: utils.cpp utils.h
	$(CXX) $(CXXFLAGS) -c utils.cpp -o utils.o

hashtable.o: hashtable.cpp hashtable.h
	$(CXX) $(CXXFLAGS) -c hashtable.cpp -o hashtable.o

server: server.cpp hashtable.o utils.o
	$(CXX) $(CXXFLAGS) server.cpp hashtable.o utils.o -o server

client: client.cpp utils.o
	$(CXX) $(CXXFLAGS) client.cpp utils.o -o client

clean:
	rm -f server client *.o
