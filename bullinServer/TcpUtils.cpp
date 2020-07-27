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
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sstream>

#define NUll_POINTER 0

int connectbyport(const char* host, const char* port) {
    return connectbyportint(host,(unsigned short)atoi(port));
}

int connectbyservice(const char* host, const char* service){
    struct servent* sinfo = getservbyname(service,"tcp");  // service info
    if (sinfo == NULL)
        return err_proto;
    return connectbyportint(host,(unsigned short)sinfo->s_port);
}


int connectbyportint(const char* host, const unsigned short port) { // host either is a DNS entre, an actual readable name, "linux.ubishops.ca" or IP address in dexcible notation "122.12.24.22"
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


int passivesocketstr(const char* port, const int backlog) {
    return passivesocket((unsigned short)atoi(port), backlog);
}

int passivesocketserv(const char* service, const int backlog) {
    struct servent* sinfo = getservbyname(service,"tcp");  // service info
    if (sinfo == NULL)
        return err_proto;
    return passivesocket((unsigned short)sinfo->s_port, backlog);
}

/*
 * Helper function, contains the common code for passivesocket and
 * controlsocket (which are identical except for the IP address they
 * bind to).
 */
int passivesockaux(const unsigned short port, const int backlog, const unsigned long int ip_addr) {
    struct sockaddr_in sin;        // address to connect to
    int    sd;                        // socket description to be returned
    const int type = SOCK_STREAM;  // TCP connection              // tcp only at the moment, SOCK_STREAM is the type for TCP, connection oriented communication

    memset(&sin, 0, sizeof(sin));  // needed for correct padding... (?),      // initiate the whole socket address structure with 0, 1st para is the pointer to a buffer(a block of memory), third para is the size of the buffer, middle para is the initialiaztion byte
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(ip_addr);

    sin.sin_port = (unsigned short)htons(port);

    // allocate socket:
    sd = socket(PF_INET, type, 0);
    if ( sd < 0 )
        return err_sock;
    
    // reusable socket   // TODO: Retest this part again,
    int reuse = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    // bind socket:
    if ( ::bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0 ) {
        close(sd);
        return err_bind;
    }
    // socket is listening:
    if ( listen(sd, backlog) < 0 ) {
        close(sd);
        return err_listen;
    }

    // done!
    return sd;
}

int passivesocket(const unsigned short port, const int backlog) {
    return passivesockaux(port, backlog, INADDR_ANY);      // INADDR_ANY if use this to the binding part, then it means you are making the socket available to all the IP for the connection
}

int  ontrolsocket(const unsigned short port, const int backlog) {
    return passivesockaux(port, backlog, INADDR_LOOPBACK);
}


// https://stackoverflow.com/questions/27494629/how-can-i-use-poll-to-accept-multiple-clients-tcp-server-c
/* Here is an example using "C" and "select" on Linux:

http://www.binarytides.com/multiple-socket-connections-fdset-select-linux/

Here is an example using "poll":

http://www-01.ibm.com/support/knowledgecenter/ssw_ibm_i_71/rzab6/poll.htm
 */


int recv_nonblock (const int sd, char* buf, const size_t max, const int timeout) {
    struct pollfd pollrec;
    pollrec.fd = sd;
    pollrec.events = POLLIN;
    
    int polled = poll(&pollrec,1,timeout);  // The poll() API allows the process to wait for an event to occur and to wake up the process when the event occurs.

    if (polled == 0)
        return recv_nodata;
    if (polled == -1)
        return -1;
    
    return (int)recv(sd,buf,max,0);
}

int readline(const int fd, char* buf, const size_t max) {
    size_t i;
    int begin = 1;

    for (i = 0; i < max; i++) {
        char tmp;
        int what = (int)read(fd,&tmp,1);    // read nbyte max from the file descriptor, fd, into buf.
        if (what == -1)
            return -1;
        if (begin) {
            if (what == 0)
                return recv_nodata;
            begin = 0;
        }
        if (what == 0 || tmp == '\n') {    //  TODO: also need to handle '\r'  ??
            buf[i] = '\0';
            return (int)i;
        }
        buf[i] = tmp;
    }
    buf[i] = '\0';
    return (int)i;
}


void printhelpFunction(void)
{
    printf("\n Usage: [OPTIONS] [Argh]\n\n");
    printf("  Options:\n");
    printf("   -b arg  (string)       overrides (or sets) the file name bbfile according to its argument\n");
    printf("   -c arg  (string)       overrides (or sets) the file name bbserv.conf  according to its argument\n");
    printf("   -T arg  (int)          overrides Tmax according to its argument\n");
    printf("   -t arg  (int)          overrides Tmax according to its argument\n");
    printf("   -p arg  (int)          overrides the port number bp (port number for client server communication) according to its argument \n");
    printf("   -s arg  (int)          overrides the port number sp (port number for server server communication) according to its argument \n");
    printf("   -f                     (with no argument) forces d to false or 0 \n");
    printf("   -d                     (with no argument) forces D to true or 1 \n");
    printf("   host:port              Any non-switch argument is further interpreted as a peer specification (and so must have the form host:port as explained above) \n");
    printf("\n");
}


int next_arg(const char* line, char delim) {
    int arg_index = 0;
   //  char msg[MAX_LEN];  // logger string

    // look for delimiter (or for the end of line, whichever happens first):
    while ( line[arg_index] != '\0' && line[arg_index] != delim)
        arg_index++;
    // if at the end of line, return -1 (no argument):
    if (line[arg_index] == '\0') {
        
        /* DEBUG_COMM */
        /*
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: next_arg(%s, %c): no argument\n", __FILE__, line ,delim);
            logger(msg);
        }
        */
        
        return -1;
    }
    // we have the index of the delimiter, we need the index of the next
    // character:
    arg_index++;
    // empty argument = no argument...
    if (line[arg_index] == '\0') {
        
        /* DEBUG_COMM */
        /*
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: next_arg(%s, %c): no argument\n", __FILE__, line ,delim);
            logger(msg);
        }
        */
        
        return -1;
    }
    
    /* DEBUG_COMM */
    /*
    if (debugs[DEBUG_COMM]) {
        snprintf(msg, MAX_LEN, "%s: next_arg(%s, %c): split at %d\n", __FILE__, line ,delim, arg_index);
        logger(msg);
    }
    */
    
    return arg_index;
}


std::string bbfileReader (std::string filename, std::string messageNumber) {
    std::string returnString = "";
    pthread_mutex_lock(&flocks.mutex);
    // We increment the number of concurrent reads, so that a write
    // request knows to wait after us.
    flocks.reads ++;
    pthread_mutex_unlock(&flocks.mutex);

    std::fstream bbFile;
    bbFile.open(filename.c_str(),std::ios::in);
    if (bbFile.fail()) {
        // we are done with the file, we first signal the condition
        // variable and then we return.
        pthread_mutex_lock(&flocks.mutex);
        flocks.reads --;
        // this might have released the file for write access, so we
        // broadcast the condition to unlock whatever writing process
        // happens to wait after us.
        if (flocks.reads == 0)
            pthread_cond_broadcast(&flocks.can_write);
        pthread_mutex_unlock(&flocks.mutex);
        return "ERROR READ can't find designated file (bbfile)";
    }
    if (D) { // debug stand out put for testing access control
       std::string logMessage = "beginning to read message \n";
       logger(&logMessage[0]);
    }
    std::string line;
    while ( getline (bbFile,line) )
    {
        std::string messageNumberInBbfile = line.substr(0, line.find("/"));
        if (messageNumber == messageNumberInBbfile) {
            size_t foundSlash= line.find_first_of('/');
            std::string posterNmessage = line.substr(foundSlash+1, line.length());
            returnString = "MESSAGE " + messageNumber + " " + posterNmessage; // notice don't return before hit the lines of code with mutex
            break;  // when found the message just get out the while and return the message
        } else {
            returnString = "UNKNOWN " + messageNumber + " can't find the designated message corresponding to this message number"; // notice don't return before hit the lines of code with mutex
        }
    }
    
    if (D) {
       // ******************** TEST CODE ********************
       // bring the read process to a crawl to figure out whether we
       // implement the concurrent access correctly.
       std::string logMessage = "debug read delay 3 seconds begins\n";
       logger(&logMessage[0]);
       sleep(3);
       std::string logMessage2 = "debug read delay 3 seconds ends\n";
       logger(&logMessage2[0]);
       // ******************** TEST CODE DONE ***************
    } /* DEBUG_DELAY */

    
    bbFile.close();
    // we are done with the file, we first signal the condition
    // variable and then we return.
    pthread_mutex_lock(&flocks.mutex);
    flocks.reads --;
    // this might have released the file for write access, so we
    // broadcast the condition to unlock whatever writing process
    // happens to wait after us.
    if (flocks.reads == 0)
        pthread_cond_broadcast(&flocks.can_write);
    pthread_mutex_unlock(&flocks.mutex);
    
    if (D) { // debug stand out put for testing access control
        std::string logMessage = "ending to read message \n";
        logger(&logMessage[0]);
    }
    
    return returnString;
}

std::string bbfileWritter (std::string filename, std::string poster, std::string message) {
    // notice don't return before hit the lines of code with mutex
    std::string returnString = "";
    
    // The fact that only one thread writes at any given time is ensured
    // by the fact that the successful thread does not release the mutex
    // until the writing process is done.
    pthread_mutex_lock(&flocks.mutex);
    // we wait for condition as long as somebody is doing things with
    // the file

    while (flocks.reads != 0) {
        // release the mutex while waiting...
        pthread_cond_wait(&flocks.can_write, &flocks.mutex);
    }
    // now we have the mutex _and_ the condition, so we write
    std::fstream bbFile;
    bbFile.open(filename.c_str(),std::ios::in | std::ofstream::app);
    if (bbFile.fail()) {
        // done writing.
        // this might have released the file for access, so we broadcast the
        // condition to unlock whatever process happens to wait for us.
        pthread_cond_broadcast(&flocks.can_write);
        // we are done!
        pthread_mutex_unlock(&flocks.mutex);
        return "ERROR WRITE can't find designated file (bbfile)"; // notice don't return before hit the lines of code with mutex
    }
    
    if (D) { // debug stand out put for testing access control
        std::string logMessage = "beginning to write message \n";
        logger(&logMessage[0]);
    }
    
    std::time_t secondsSince1970 = std::time(0);
    std::stringstream ss;
    ss << secondsSince1970;
    std::string uniqueMessageNumber = ss.str();
    bbFile << uniqueMessageNumber << "/" << poster << "/" << message << std::endl;
    returnString = "WROTE " + uniqueMessageNumber;
    
    if (D) { /* DEBUG_DELAY */
        // ******************** TEST CODE ********************
        // bring the write process to a crawl to figure out whether we
        // implement the concurrent access correctly.
        std::string logMessage = "debug write delay 6 seconds begins \n";
        logger(&logMessage[0]);
        sleep(6);
        std::string logMessage2 = "debug write delay 6 seconds ends \n";
        logger(&logMessage2[0]);
        // ******************** TEST CODE DONE ***************
    }
    
    bbFile.close();
    // done writing.
    // this might have released the file for access, so we broadcast the
    // condition to unlock whatever process happens to wait for us.

    pthread_cond_broadcast(&flocks.can_write);
    // we are done!
    pthread_mutex_unlock(&flocks.mutex);

    if (D) { // debug stand out put for testing access control
       std::string logMessage = "end to write message \n";
       logger(&logMessage[0]);
    }
       
    return returnString;
}

std::string bbfileReplacer (std::string filename, std::string messageNumber, std::string newMessage, std::string newPoster) {
   
    // The fact that only one thread writes at any given time is ensured
    // by the fact that the successful thread does not release the mutex
    // until the writing process is done.
    pthread_mutex_lock(&flocks.mutex);
    // we wait for condition as long as somebody is doing things with
    // the file
    while (flocks.reads != 0) {
        // release the mutex while waiting...
        pthread_cond_wait(&flocks.can_write, &flocks.mutex);
    }
    // now we have the mutex _and_ the condition, so we write
    
    // notice don't return before hit the lines of code with mutex
    std::string returnString = "";
    
    if (D) { // debug stand out put for testing access control
        std::string logMessage = "beginning to replace message \n";
        logger(&logMessage[0]);
    }
    
    std::fstream bbFile;
    bbFile.open(filename.c_str(),std::ios::in | std::ofstream::app);
    if (bbFile.fail()) {
        // done writing.
        // this might have released the file for access, so we broadcast the
        // condition to unlock whatever process happens to wait for us.
        pthread_cond_broadcast(&flocks.can_write);
        // we are done!
        pthread_mutex_unlock(&flocks.mutex);
        return "ERROR WRITE can't find designated file (bbfile)"; // notice don't return before hit the lines of code with mutex
    }
    
    std::string bbfileContainer = "";
    
    std::string line;
    bool foundMessageNumber = false;
    while ( getline (bbFile,line) )
    {
        std::string messageNumberInBbfile = line.substr(0, line.find("/"));
        if (messageNumber == messageNumberInBbfile) {
            line.clear();
            line = messageNumber + "/" + newPoster + "/" + newMessage;
            foundMessageNumber = true;
        }
        bbfileContainer = bbfileContainer + line + "\n";
    }
    if (foundMessageNumber) {
        returnString = "WROTE " + messageNumber;  // notice don't return before hit the lines of code with mutex
        foundMessageNumber = false;
    } else {
        returnString = "UNKNOWN " + messageNumber + " can't find the designated message corresponding to this message number"; // notice don't return before hit the lines of code with mutex
    }
    bbFile.close();
    
    bbFile.open(filename.c_str(),std::ios::out);  // replace all the text in the text file again
    if (bbFile.fail()) {
        returnString = "Unable to find defaultConfig file 2";
        // exit(0);
    }
    bbFile << bbfileContainer;
    bbFile.close();
    
    if (D) { /* DEBUG_DELAY */
        // ******************** TEST CODE ********************
        // bring the write process to a crawl to figure out whether we
        // implement the concurrent access correctly.
        std::string logMessage = "debug replace delay 6 seconds begins \n";
        logger(&logMessage[0]);
        sleep(6);
        std::string logMessage2 = "debug replace delay 6 seconds ends \n";
        logger(&logMessage2[0]);
        // ******************** TEST CODE DONE ***************
    }
    
    // done writing.
    // this might have released the file for access, so we broadcast the
    // condition to unlock whatever process happens to wait for us.
    pthread_cond_broadcast(&flocks.can_write);
    // we are done!
    pthread_mutex_unlock(&flocks.mutex);
    
    if (D) { // debug stand out put for testing access control
       std::string logMessage = "end to replace message \n";
       logger(&logMessage[0]);
    }
       
    return returnString;
}

TcpUtils* TcpUtils::singleton = NUll_POINTER;
TcpUtils* TcpUtils::newAInstance() {
    if (singleton == NUll_POINTER) {
        singleton = new TcpUtils;
    }
    return singleton;
}
