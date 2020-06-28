/*
 * Simple multithreaded client which repeatedly receives lines from
 * clients and return them prefixed by the string "ACK: ".  Release
 * the connection with a client upon receipt of the string "quit"
 * (case sensitive) or upon a connection close by the client.
 *
 * This version featured a monitor thread, which wakes up each two
 * minutes and reports some information.  The monitoring process is
 * the same as the one used in the textbook.
 *
 * By Stefan Bruda
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "tcp-utils.h"

/*
 * Global variables, for monitoring.  Notice that it is global, and
 * thus visible in all the threads; which is good, because all the
 * client threads will want to write to it so that the monitor can
 * report meaningful information.
 */

struct monitor_t {
    pthread_mutex_t mutex;
    unsigned int con_cur;
    unsigned int con_count;
    unsigned int con_time;
    unsigned int bytecount;
};

// Monitor statistics:
monitor_t mon;

/*
 * The function that implements the monitor thread.
 */
void* monitor (void* ignored) {
    const int wakeup_interval = 120; // 2 minutes
    int connections;
    
    while (1) {
        sleep(wakeup_interval);
        pthread_mutex_lock(&mon.mutex);
        time_t now = time(0);
        if (mon.con_count == 0)
            connections = 1;
        else
            connections = mon.con_count;
        printf("MON: %s\n", ctime(&now));
        printf("MON: currently serving %d clients\n", mon.con_cur);
        printf("MON: average connection time is %d seconds.\n",
               (int)((float)mon.con_time/(float)connections));
        printf("MON: transferred %d bytes per connection on average.\n",
               (int)((float)mon.bytecount/(float)connections));
        printf("MON: (end of information)\n");
        pthread_mutex_unlock(&mon.mutex);
    }
    return 0;
}

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
    time_t start = time(0);

    printf("Incoming client...\n");
    // monitor code begins
    pthread_mutex_lock(&mon.mutex);                    //  when u define a mutex, you will define a critical region
    mon.con_cur++;
    pthread_mutex_unlock(&mon.mutex);
    // monitor code ends

    // Loop while the client has something to say...
    while ((n = readline(sd,req,ALEN-1)) != recv_nodata) {
        if (strcmp(req,"quit") == 0) {
            printf("Received quit, sending EOF.\n");
            shutdown(sd,1);
            close(sd);
            printf("Outgoing client...\n");
            // monitor code begins
            pthread_mutex_lock(&mon.mutex);
            mon.con_cur--;
            mon.con_count++;
            mon.con_time += time(0) - start;
            pthread_mutex_unlock(&mon.mutex);
            // monitor code ends
            return NULL;
        }
        // monitor code begins
        pthread_mutex_lock(&mon.mutex);
        mon.bytecount += n;
        pthread_mutex_unlock(&mon.mutex);
        // monitor code ends
        send(sd,ack,strlen(ack),0);
        send(sd,req,strlen(req),0);
        send(sd,"\n",1,0);
    }
    // read 0 bytes = EOF:
    printf("Connection closed by client.\n");
    shutdown(sd,1);
    close(sd);
    printf("Outgoing client...\n");
    // monitor code begins
    pthread_mutex_lock(&mon.mutex);
    mon.con_cur--;
    mon.con_count++;
    mon.con_time += time(0) - start;
    pthread_mutex_unlock(&mon.mutex);
    // monitor code ends
    return NULL;
}

int main (int argc, char** argv) {
    const int port = 9004;   // port to listen to
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

    // Launch the monitor:
    pthread_create(&tt, &ta, monitor, NULL); 

    while (1) {
        // Accept connection:
        ssock = accept(msock, (struct sockaddr*)&client_addr, &client_addr_len);   //  the return value is a socket
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
