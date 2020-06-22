#include <stdio.h>  
#include <string.h>   
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>

#define SERVER_PORT 8082
     
int main(int argc , char *argv[])   
{   
    int opt = 1;   
    int msc , addlen , newSock , cliSock[30] ,  
          maxClients = 30 , ac, i , val , sd;   
    int msd;   
    struct sockaddr_in addr;   
         
    char buf[1025];  
         
    fd_set rfds;   
    char *msg = "";   
     
    for (i = 0; i < maxClients; i++)   
    {   
        cliSock[i] = 0;   
    }   
         
    if( (msc = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket connection failed");   
        exit(EXIT_FAILURE);   
    }   
     
    if( setsockopt(msc, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setting sock opt");   
        exit(EXIT_FAILURE);   
    }   
     
    addr.sin_family = AF_INET;   
    addr.sin_addr.s_addr = INADDR_ANY;   
    addr.sin_port = htons( SERVER_PORT );   
         
    if (bind(msc, (struct sockaddr *)&addr, sizeof(addr))<0)   
    {   
        perror("server build failure");   
        exit(EXIT_FAILURE);   
    }   
    printf("Server listing on port %d \n", SERVER_PORT);   
         
    if (listen(msc, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    addlen = sizeof(addr);   
    puts("Server Waiting for connections on 8082");   
         
    while(1)   
    {   
        FD_ZERO(&rfds);   
     
        FD_SET(msc, &rfds);   
        msd = msc;   
             
        for ( i = 0 ; i < maxClients ; i++)   
        {   
            sd = cliSock[i];   
                 
            if(sd > 0)   
                FD_SET( sd , &rfds);   
                 
            if(sd > msd)   
                msd = sd;   
        }   
     
        ac = select( msd + 1 , &rfds , NULL , NULL , NULL);   
       
        if ((ac < 0) && (errno!=EINTR))   
        {   
            printf("Eeror");   
        }   
             
        if (FD_ISSET(msc, &rfds))   
        {   
            if ((newSock = accept(msc,  
                    (struct sockaddr *)&addr, (socklen_t*)&addlen))<0)   
            {   
                perror("accepted ");   
                exit(EXIT_FAILURE);   
            }   
             
            printf("Arraived new connection , server socket descp is %d , ip is : %s , SERVER_PORT : %d  \n" , newSock , inet_ntoa(addr.sin_addr) , ntohs (addr.sin_port));   
           
            if( send(newSock, msg, strlen(msg), 0) != strlen(msg) )   
            {   
                perror("send");   
            }   
                 
            puts("Message delivered succesfully");   
                 
            for (i = 0; i < maxClients; i++)   
            {   
                if( cliSock[i] == 0 )   
                {   
                    cliSock[i] = newSock;   
                    printf("Inserting list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
             
        for (i = 0; i < maxClients; i++)   
        {   
            sd = cliSock[i];   
                 
            if (FD_ISSET( sd , &rfds))   
            {   
                if ((val = read( sd , buf, 1024)) == 0)   
                {   
                    getpeername(sd , (struct sockaddr*)&addr , \ 
                        (socklen_t*)&addlen);   
                    printf("Client disconnected , ip %s , SERVER_PORT %d \n" ,  
                          inet_ntoa(addr.sin_addr) , ntohs(addr.sin_port));   
                         
                    close( sd );   
                    cliSock[i] = 0;   
                }   
                     
                else 
                {   
                    buf[val] = '\0';   
                    send(sd , buf , strlen(buf) , 0 );   
                }   
            }   
        }   
    }   
         
    return 0;   
}   
