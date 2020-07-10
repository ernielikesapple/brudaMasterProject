//
//  TcpUtils.cpp
//  bullinServer
//
//  Created by Ernie on 2020-07-08.
//  Copyright © 2020 Ernie. All rights reserved.
//

#include "TcpUtils.hpp"
#include "fstream"
#include <iostream>


using namespace std;

#define NUll_POINTER 0

TcpUtils* TcpUtils::singleton = NUll_POINTER;


TcpUtils* TcpUtils::newAInstance() {
    if (singleton == NUll_POINTER) {
        singleton = new TcpUtils;
    }
    return singleton;
}

int TcpUtils::connectbyportint(const char* host, const unsigned short port) { // host either is a DNS entre, an actual readable name, "linux.ubishops.ca" or IP address in dexcible notation "122.12.24.22"
    struct hostent *hinfo;         // host information,              convert the DNS name into real decimal number
    struct sockaddr_in sin;        // address to connect to          assemble all the infomation together for indentifying the peer
    int sd;                        // socket descriptor to be returned
    const int type = SOCK_STREAM;  // TCP connection

    memset(&sin, 0, sizeof(sin));  // needed for correct padding... (?)  // initialize the sockaddr_in Struct
    sin.sin_family = AF_INET;

    // host name to IP address
    hinfo = gethostbyname(host);
    if (hinfo == NULL)
        return err_host;
    
    memcpy(&sin.sin_addr, hinfo->h_addr, hinfo->h_length); //para1 An array where s2 will be copied to.   para2 The string to be copied.  p3 The number of characters to copy.
    /*  difference between memset, memcpy, memmove
     memset sets a block of memory to a single value. memcpy copies the content of a block into another block.  Perhaps you'd be interested in the difference between memcpy and memmove. Both do the same, but the latter works even if the source and destination overlap.
     */
    
    
    sin.sin_port = (unsigned short)htons(port); // 🌟 notice, the difference, in the local machine, le numero de port est en form de  Little Endianet, et in the server is Big endian ,  htons. host to network, convert host endian to network endian, this function works like, if the machine is big endian, then this function will not work, if not, then this function will convert the local little endian format to the big endian format.,  htons and htonl, host to network short, host to network long
    
    // allocate socket:  for the connecting to the server
    sd = socket(PF_INET, type, 0);
    /*
     int socket(int domain, int type, int protocol)

     Used to create a new socket, returns a file descriptor for the socket or -1 on error.
     It takes three parameters:
     domain: the protocol family of socket being requested
     type: the type of socket within that family
     and the protocol.

     The parameters allow us to say what kind of socket we want (IPv4/IPv6, stream/datagram(TCP/UDP)).
     The protocol family should be AF_INET or AF_INET6
     and the protocol type for these two families is
     either SOCK_STREAM for TCP/IP or SOCK_DGRAM for UDP/IP.
     The protocol should usually be set to zero to indicate that the default protocol should be used.
     */
    
    if ( sd < 0 )
        return err_sock;
    
    // connect socket:
    int rc = connect(sd, (struct sockaddr *)&sin, sizeof(sin));     // 1st para is the socket discriptor, 2nd 3nd are the coordinator of the server, coordianates are struct, which contains the ip address and port number. and 3rd is the size of the struct
    
    /* difference between sockaddr and sockaddr_in
     sockaddr is a generic descriptor for any kind of socket operation, whereas sockaddr_in is a struct specific to IP-based communication (IIRC, "in" stands for "InterNet"). As far as I know, this is a kind of "polymorphism" : the bind() function pretends to take a struct sockaddr *, but in fact, it will assume that the appropriate type of structure is passed in; i. e. one that corresponds to the type of socket you give it as the first argument.
     
     */
    if (rc < 0) {
        close(sd);
        return err_connect;
    }

    // done!
    return sd;
}


void TcpUtils::printhelpFunction(void)
{
    printf("\n Usage: [OPTIONS] [Argh]\n\n");
    printf("  Options:\n");
    printf("   -b arg  (string)       overrides (or sets) the file name bbfile according to its argument\n");
    printf("   -c arg  (string)       overrides (or sets) the file name bbserv.conf  according to its argument\n");
    printf("   -T arg  (int)          overrides Tmax according to its argument\n");
    printf("   -t arg  (int)          overrides Tmax according to its argument\n");
    printf("   -p arg  (int)          overrides the port number bp according to its argument \n");
    printf("   -s arg  (int)          overrides the port number sp according to its argument \n");
    printf("   -f                     (with no argument) forces d to false or 0 \n");
    printf("   -d                     (with no argument) forces D to true or 1 \n");
    printf("   host:port              Any non-switch argument is further interpreted as a peer specification (and so must have the form host:port as explained above) \n");
    printf("\n");
}