CXX = g++
CXXFLAGS = -g -pedantic -Wall -Werror

all: client

## Common:
tcp-utils.o: tcp-utils.h tcp-utils.cc
	$(CXX) $(CXXFLAGS) -c -o tcp-utils.o tcp-utils.cc

## Client:
client.o: tcp-utils.h client.cc
	$(CXX) $(CXXFLAGS) -c -o client.o client.cc

client: client.o tcp-utils.o
	$(CXX) $(CXXFLAGS) -o client client.o tcp-utils.o

clean:
	rm -f client *~ *.o *.bak core \#*

