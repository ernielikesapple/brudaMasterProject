/*
 * Simple multithreaded client which repeatedly receives lines from
 * clients and return them prefixed by the string "ACK: ".  Release
 * the connection with a client upon receipt of the string "quit"
 * (case sensitive) or upon a connection close by the client.
 *
 * By Stefan Bruda
 */
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include "tcp-utils.h"

/*
 * Repeatedly receives requests from the client and responds to them.
 * If the received request is an end of file or "quit", terminates the
 * connection.  Note that an empty request also terminates the
 * connection.  Same as for the purely iterative or the multi-process
 * server.
 */
void* do_client (int sd) {
    const int ALEN = 256;
    char req[ALEN];
    const char* ack = "ACK: ";
    int n;
    
    printf("Incoming client...\n");
    
    // Loop while the client has something to say...
    while ((n = readline(sd,req,ALEN-1)) != recv_nodata) {
        if (strcmp(req,"quit") == 0) {
            printf("Received quit, sending EOF.\n");
            shutdown(sd,1);
            close(sd);
            printf("Outgoing client...\n");
            return NULL;
        }
        send(sd,ack,strlen(ack),0);
        send(sd,req,strlen(req),0);
        send(sd,"\n",1,0);
    }
    // read 0 bytes = EOF:
    printf("Connection closed by client.\n");
    shutdown(sd,1);
    close(sd);
    printf("Outgoing client...\n");
    return NULL;
}

int main (int argc, char** argv) {
    const int port = 9003;   // port to listen to
    const int qlen = 32;     // incoming connections queue length
    
    // Note that the following are local variable, and thus not shared
    // between threads; especially important for ssock and client_addr.
    
    long int msock, ssock;               // master and slave socket
  
    struct sockaddr_in client_addr; // the address of the client...
    unsigned int client_addr_len = sizeof(client_addr); // ... and its length
    
    msock = passivesocket(port,qlen);
    if (msock < 0) {
        perror("passivesocket");
        return 1;
    }
    printf("Server up and listening on port %d.\n", port);

    // Setting up the thread creation:
    pthread_t tt;
    pthread_attr_t ta;
    pthread_attr_init(&ta);
    pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);
    
    while (1) {
        // Accept connection:
        ssock = accept(msock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (ssock < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            return 1;
        }
        
        // Create thread instead of fork:
        if ( pthread_create(&tt, &ta, (void* (*) (void*))do_client, (void*)ssock) != 0 ) {
            perror("pthread_create");
            return 1;
        }
        // main thread continues with the loop...
    }
    return 0;   // will never reach this anyway...
}
