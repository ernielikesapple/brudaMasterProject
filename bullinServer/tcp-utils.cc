/*
 * Miscelanious TCP (and general) functions.  Contains functions that
 * connect active sockets, put passive sockets in listening mode, and
 * read from streams.
 *
 * By Stefan Bruda, using the textbook as inspiration.
 */

#include "tcp-utils.h"

int connectbyport(const char* host, const char* port) {
    return connectbyportint(host,(unsigned short)atoi(port));
}

int connectbyservice(const char* host, const char* service){
    struct servent* sinfo = getservbyname(service,"tcp");  // service info
    if (sinfo == NULL)
        return err_proto;
    return connectbyportint(host,(unsigned short)sinfo->s_port);
}

// 🌟
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
    int	sd;                        // socket description to be returned
    const int type = SOCK_STREAM;  // TCP connection              // tcp only at the moment, SOCK_STREAM is the type for TCP, connection oriented communication

    memset(&sin, 0, sizeof(sin));  // needed for correct padding... (?),      // initiate the whole socket address structure with 0, 1st para is the pointer to a buffer(a block of memory), third para is the size of the buffer, middle para is the initialiaztion byte
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(ip_addr);

    sin.sin_port = (unsigned short)htons(port);

    // allocate socket:
    sd = socket(PF_INET, type, 0);
    if ( sd < 0 )
        return err_sock;
    
    // bind socket:
    if ( bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0 ) {
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
    return passivesockaux(port, backlog, INADDR_ANY);
}

int controlsocket(const unsigned short port, const int backlog) {
    return passivesockaux(port, backlog, INADDR_LOOPBACK);
}

int recv_nonblock (const int sd, char* buf, const size_t max, const int timeout) {
    struct pollfd pollrec;
    pollrec.fd = sd;
    pollrec.events = POLLIN;
    
    int polled = poll(&pollrec,1,timeout);

    if (polled == 0)
        return recv_nodata;
    if (polled == -1)
        return -1;
    
    return recv(sd,buf,max,0);
}

int readline(const int fd, char* buf, const size_t max) {
    size_t i;
    int begin = 1;

    for (i = 0; i < max; i++) {
        char tmp;
        int what = read(fd,&tmp,1);
        if (what == -1)
            return -1;
        if (begin) {
            if (what == 0)
                return recv_nodata;
            begin = 0;
        }
        if (what == 0 || tmp == '\n') {
            buf[i] = '\0';
            return i;
        }
        buf[i] = tmp;
    }
    buf[i] = '\0';
    return i;
}
