
CXX = clang++

#CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined 

CXXFLAGS = -std=c++17 -Wall -Wextra -Werror 

#CXXFLAGS += -g

all: client server test_avl

client: client.cpp utils.o
	$(CXX) $(CXXFLAGS) client.cpp utils.o -o client

#hashtable.a: hashtable.o
#	ar rcs hashtable.a hashtable.o

hashtable.o: hashtable.cpp hashtable.h
	$(CXX) $(CXXFLAGS)  -c hashtable.cpp -o hashtable.o

utils.o: utils.cpp utils.h
	$(CXX) $(CXXFLAGS) -c utils.cpp -o utils.o

avl.o: avl.cpp avl.h
	$(CXX) $(CXXFLAGS) -c avl.cpp -o avl.o

test_avl: test_avl.cpp avl.o
	$(CXX) $(CXXFLAGS) test_avl.cpp  avl.o -o test

server: server.cpp hashtable.o utils.o
	$(CXX) $(CXXFLAGS) server.cpp hashtable.o utils.o -o server

clean:
	rm -f *.o server client *.dSYM
