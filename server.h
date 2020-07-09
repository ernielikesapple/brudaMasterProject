

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>

#include "tcp-utils.h"






typedef enum {
    idle = 1,
    running  = 2,
    recreate  = 3,
    deactivate = 4
}thread_status;


struct client_t {
    int sd;    
    char ip[20];   
    bool is_server_busy;
    pthread_t thread;
    pthread_cond_t thread_cond;
    int status;
    pthread_mutex_t lock;
    int shutdown;
};




struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  int t_incr;
  int max_size;
  int active;
  int count;
  int shutdown;
  int started;
  client_t* clients;
};



struct replica_server{
  int sd;    
  char *ip;  
  int port;
};




extern const char* logfile;


const size_t MAX_LEN = 1024;

int next_arg(const char*, char);


const size_t DEBUG_COMM = 0;
const size_t DEBUG_FILE = 1;
const size_t DEBUG_DELAY = 2;
extern bool debugs[3]; 

void logger(const char *);


extern pthread_mutex_t logger_mutex;



struct rwexcl_t {    
    pthread_mutex_t mutex;     
    pthread_cond_t can_write;   
    unsigned int reads;               
    unsigned int owners;        
    int fd;                                          
    char* name;                 
};


extern rwexcl_t** flocks;
extern size_t flocks_size;


const int err_nofile = -2;

void* file_client (client_t*);



void* replica_sync_request_recieve(int);

void update_replica_servers(replica_server*);

void sync_with_server(char*);




const int err_exec = 0xFF;
