CXX = g++
CXXFLAGS = -g #-Wall -Werror   #usage??

all: bbserv

## Common:
tcp-utils.o: tcp-utils.h tcp-utils.cc
	$(CXX) $(CXXFLAGS) -c -o tcp-utils.o tcp-utils.cc

## Server:
main.o: main.cpp tcp-utils.h #add all the dependency
	$(CXX) $(CXXFLAGS) -c -o main.o main.cpp

bbserv: main.o tcp-utils.o
	$(CXX) $(CXXFLAGS) -o bbserv main.o tcp-utils.o

clean:
	rm -f iserv ciserv cserv *~ *.o *.bak core \#*

