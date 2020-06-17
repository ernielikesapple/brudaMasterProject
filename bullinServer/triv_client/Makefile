CXX = g++
CXXFLAGS = -g -pedantic -Wall -Werror

all: triv_client

## Common:
tcp-utils.o: tcp-utils.h tcp-utils.cc
	$(CXX) $(CXXFLAGS) -c -o tcp-utils.o tcp-utils.cc

## Client:
client.o: tcp-utils.h client.cc
	$(CXX) $(CXXFLAGS) -c -o client.o client.cc

triv_client: client.o tcp-utils.o
	$(CXX) $(CXXFLAGS) -o triv_client client.o tcp-utils.o

clean:
	rm -f triv_client *~ *.o *.bak core \#*

