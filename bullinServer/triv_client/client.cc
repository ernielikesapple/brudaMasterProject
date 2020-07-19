/*
 * Simple parameterized, text-only TCP client.  Expects on the command
 * line a host name and a post number and connects to those
 * coordinates.  It then takes lines of text from the standard input,
 * sends them to the server, printing back the server's respone.
 *
 * By Stefan Bruda
 */
#include <libgen.h>
#include <stdio.h>

#include "tcp-utils.h"

int main (int argc, char** argv) {
    const int ALEN = 256;
    char requestMessage[ALEN];
    char answerFromServer[ALEN];
    
    if (argc != 3) {
        printf("please use the correct input format->     Usage: %s host port\n", basename(argv[0]));
        return 1;
    }

    int sd = connectbyport(argv[1],argv[2]);   // host and port number
    if (sd == err_host) {
        fprintf(stderr, "Cannot find host %s.\n", argv[1]);
        return 1;
    }
    if (sd < 0) {
        perror("connectbyport");
        return 1;
    }
    // we now have a valid, connected socket
    printf("Connected to %s on port %s.\n", argv[1], argv[2]);

    while (1) {
        int n;
        
        while ((n = recv_nonblock(sd,answerFromServer,ALEN-1,500)) != recv_nodata) {
            if (n == 0) {
                shutdown(sd, SHUT_RDWR);
                close(sd);
                printf("Connection closed by %s.\n", argv[1]);
                return 0;
            }
            if (n < 0) {
                perror("recv_nonblock");
                shutdown(sd, SHUT_WR);
                close(sd);
                break;
            }
            answerFromServer[n] = '\0';
            printf("%s", answerFromServer);
            fflush(stdout);
        }

        printf("> ");
        fflush(stdout);
        fgets(requestMessage,256,stdin);
        // eat up the terminating newline
        if(strlen(requestMessage) > 0 && requestMessage[strlen(requestMessage) - 1] == '\n')
            requestMessage[strlen(requestMessage) - 1] = '\0';
        fflush(stdout);
        
        send(sd,requestMessage,strlen(requestMessage),0);
        send(sd,"\n",1,0);
    }
}
