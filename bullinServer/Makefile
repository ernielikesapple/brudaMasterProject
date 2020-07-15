CXX = g++
CXXFLAGS = -std=c++11 -c #-Wall #-Werror   #usage??
LDFLAGS = -lpthread

all: bbserv

bbserv: main.o TcpUtils.o ConfigFileHandler.o
	$(CXX) $(LDFLAGS) main.o TcpUtils.o ConfigFileHandler.o -o bbserv

main.o: main.cpp #add all the dependency
	$(CXX) $(CXXFLAGS) main.cpp

TcpUtils.o: TcpUtils.cpp
	$(CXX) $(CXXFLAGS) TcpUtils.cpp

ConfigFileHandler.o: ConfigFileHandler.cpp
	$(CXX) $(CXXFLAGS) ConfigFileHandler.cpp

clean:
	rm -rf *o bbserv \

