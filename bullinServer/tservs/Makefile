CXX = g++ 
CC = g++
CXXFLAGS = -g -Wall -Werror -ansi -pedantic
LDFLAGS = -lpthread

all: tserv mtserv

## Common:
tcp-utils.o: tcp-utils.h tcp-utils.cc

## multithreaded
tserv.o: tcp-utils.h tserv.cc
tserv: tserv.o tcp-utils.o

## multithreaded, with a monitor
mtserv.o: tcp-utils.h mtserv.cc
mtserv: mtserv.o tcp-utils.o

clean:
	rm -f mtserv tserv *~ *.o *.bak core \#*
