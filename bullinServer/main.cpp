#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>

#include "TcpUtils.hpp"
#include "ConfigFileHandler.hpp"

char const* app_name = "bbserv";
std::string configFileName = "bbserv.conf";
std::string Tmax = "20";   // keyValue in the config file: THMAX   // use stoi(), change int to string
std::string bp = "9000";   // keyValue in the config file: BBPORT , port number for client server communication
std::string sp = "10000";  // keyValue in the config file: SYNCPORT,  port number for server server communication
std::string bbfile = "bbfile";   // keyValue in the config file: BBFILE               // config fileName mandatory data if it's empty then refuse to start the server,
std::string peers = "";    // keyValue in the config file: PEERS // TODO: to verify the value inside the server function make sure it has value
bool d = true;   // keyValue in the config file: DAEMON
bool D = false; // keyValue in the config file: DEBUG

char const* PIDFile = "bbserv.pid"; // the lock file, to lock the critical region so that only one daemon is runnning at a time
static int PIDFileDescriptor = -1;

char const* bbservLogFile = "bbserv.log"; // redirct all the output to this file when under daemon mode



// server para
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

static int running = 0;

// all the singleTon
TcpUtils* tcpUtils = TcpUtils::newAInstance();
ConfigFileHandler* configFileHandler = ConfigFileHandler::newAInstance();

void loadConfigFile();
void daemonize();
void signalHandlers(int sig);

/*
* The function that implements the monitor thread.
*/
void* monitor (void* ignored);
void* do_client (int sd);

int main(int argc, char** argv) {
    
    loadConfigFile();
    
    umask(0022); // Sets an appropriate umask.
    
    // handle command line arguments:
    int option;
    std::string lastInput = "";

    // Retrieve the (non-option) argument:
    if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') ) {  // there is NO input...
        // cerr << "No argument provided!" << endl;
        // handle the situation when there is no arguments provided
    }
    else {  // there is an input...
        lastInput = argv[argc-1];
    }
    
    // Retrieve the options:
    std::string peersString = "";
    while (optind < argc) {
        if ( (option = getopt(argc, argv, "b:c:T:t:p:s:fdh")) != -1 ) {  // for each option...
            switch ( option ) {
                case 'b':  // change bbfile name
                    std::rename(bbfile.c_str(), optarg);
                    configFileHandler -> configFileModifier(configFileName,"BBFILE",optarg); // insert the new name inside the config file
                    bbfile = optarg;
                    break;
                case 'c':  // change config file name
                    std::rename(configFileName.c_str(), optarg);
                    configFileName = optarg; // TODO: VERIFY THIS with the signalhug part again insert the new value into this configFile variable so that when reload it will use the new name
                    break;
                case 'T':  // overide Tmax number, preallocated threads
                    configFileHandler -> configFileModifier(configFileName,"THMAX",optarg);
                    Tmax = optarg;
                    break;
                case 't':  // overide Tmax number, preallocated threads
                    configFileHandler -> configFileModifier(configFileName,"THMAX",optarg);
                    Tmax = optarg;
                    break;
                case 'p':  // client server port number
                    configFileHandler -> configFileModifier(configFileName,"BBPORT",optarg);
                    bp = optarg;
                    break;
                case 's': // server port number
                    configFileHandler -> configFileModifier(configFileName,"SYNCPORT",optarg);
                    sp = optarg;
                    break;
                case 'f':   // start daemon...
                    configFileHandler -> configFileModifier(configFileName,"DAEMON","false");
                    d = false;
                    break;
                case 'd':   // debug mode...
                    configFileHandler -> configFileModifier(configFileName,"DEBUG","true");
                    D = true;
                    break;
                case '?':  // unknown option...
                        std::cerr << "Unknown option: '" << char(optopt) << "'!" << std::endl;
                    break;
                case 'h':  // help menu
                    tcpUtils -> printhelpFunction();
                default:
                    break;
            }
        } else {
            // TODO: use a string to append/take all the values from argv[optind] , and then after the while loop take everything into the config file
            std::cout << "handle non-switch argument are: " << argv[optind] << std::endl;
            if (strcmp(argv[optind], "") != 0) {
                peersString = peersString + argv[optind] + " ";
            }
            optind++;
        }
    }
    
    // TODO: handle,       check and see other start options check the documents again see if there is any other switch options
    if (peersString.length() != 0) {
        configFileHandler -> configFileModifier(configFileName,"PEERS",peersString);
    }
    
    loadConfigFile();  // reload everything after config
    // TODO: change the value in the config file if the value for d is true then we need to start the server
    if (d) { // daemonizing
        daemonize();
    }
       
    /* Open system log and write message to it */
    openlog(argv[0], LOG_PID|LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started %s", app_name);
    
    signal(SIGQUIT,signalHandlers); // Installs appropriate signal handlers for all the signals.
    signal(SIGHUP,signalHandlers);
    
    // TODO: START THE SERVER
    running = 1;
    while (running == 1) {
        // insert server code here
        
        
        
        
        const int port = 9004;   // port to listen to
        const int qlen = 32;     // incoming connections queue length

        // Note that the following are local variable, and thus not shared
        // between threads; especially important for ssock and client_addr.

        long int msock, ssock;               // master and slave socket

        struct sockaddr_in client_addr; // the address of the client...
        unsigned int client_addr_len = sizeof(client_addr); // ... and its length

        msock = tcpUtils -> passivesocket(port,qlen);
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
          ssock = ::accept(msock, (struct sockaddr*)&client_addr, &client_addr_len);   //  the return value is a socket
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
        
        
        
        
        
        
    }
    
    // TODO: HANDLE: END THE SERVER
    
    /* Write system log and close it. */
    syslog(LOG_INFO, "Stopped %s", app_name);
    closelog();
    // Free allocated memory
    //  if (PIDFile !=  NULL) {  delete PIDFile; }
    
    return 0;
}

// detail implementation of each function
void loadConfigFile() {
    if (std::ifstream(configFileName)) { // config file exists
        configFileHandler ->configFileReader(configFileName); // read all the value from config file and load them in a map inside configFileHandler
        
        configFileHandler ->configFileValueGetter("THMAX", Tmax);  // get all the value from the map into all the variabbles on this page
        configFileHandler ->configFileValueGetter("BBPORT", bp);
        configFileHandler ->configFileValueGetter("SYNCPORT", sp);
        configFileHandler ->configFileValueGetter("BBFILE", bbfile);
        if (!std::ifstream(bbfile)) { // make sure the bbfile exist, so the server can start up normally
            std::cerr << "bbfile Not provided! Notice you must have a bbfile under current directory and Notice you should name the bbfile file like 'bbfile' " << std::endl;
            exit(0);
        }
        configFileHandler ->configFileValueGetter("PEERS", peers); // TODO: to verify the value inside the server function make sure it has value
        
        std::string tempStringBoolean_d = "";
        configFileHandler ->configFileValueGetter("DAEMON", tempStringBoolean_d);

        if (tempStringBoolean_d == "true" || tempStringBoolean_d == "TRUE" || tempStringBoolean_d == "True" || tempStringBoolean_d == "1") {
            d = true;
        } else {
            d = false;
        }
        
        std::string tempStringBoolean_D = "";
        configFileHandler ->configFileValueGetter("DEBUG", tempStringBoolean_D);
        if (tempStringBoolean_D == "true" || tempStringBoolean_D == "TRUE" || tempStringBoolean_D == "True" || tempStringBoolean_D == "1") {
            D = true;
        } else {
            D = false;
        }
    } else {
        std::cerr << "config file Not provided! Notice you should name the config file like 'bbserv.conf' " << std::endl;
        exit(0);
    }
}

void signalHandlers(int sig) { //TODO: Handle all the signals
    if (sig == SIGQUIT) { // Quit the daemon
        // TODO: Closes all the master sockets
        // TODO: terminates all the preallocated threads
        // TODO: closes all the connections to all the clients
        // TODO: terminates the server
        
        /* Unlock and close lockfile */
        if (PIDFileDescriptor != -1) {
            lockf(PIDFileDescriptor, F_ULOCK, 0);
            close(PIDFileDescriptor);
        }
        /* Try to delete lockfile */
        if (PIDFile != NULL) {
            remove(PIDFile);
        }
        
        running = 0;
        /* Reset signal handling to default behavior */
        signal(SIGINT, SIG_DFL);
    } else if (sig == SIGHUP) {
        // TODO: Closes all the master sockets
        // TODO: terminates all the preallocated threads
        // TODO: closes all the connections to all the clients
        loadConfigFile(); // reload the config file
        // TODO: RESTART THE SERVER PART
        
    } else if (sig == SIGCHLD) {
        // TODO: handle SIGCHLD
    }
}

void daemonize() {
    
    int bgpid = fork();
    if (bgpid < 0) {
        perror("startup fork");
        exit(EXIT_FAILURE);
    }
    
    if (bgpid) { // Leaves the current process group.
        exit(EXIT_SUCCESS);
    }
    
    signal(SIGCHLD, SIG_IGN);


    /* Set new file permissions */
    // umask(0);
    
    /* Change the working directory to the root directory */
    // chdir("/");
    // chdir("/run/bbserv"); TODO: CHECK why this is not creating folder under /run/bbserv
    
    // Closes all file descriptors and re-opens them as appropriate. In particular console output is redirected to the file bbserv.log located in the current working directory.
    for (int i = getdtablesize() - 1; i >= 0 ; i--) {
        close(i);
    }
    
    // Detaches from the controlling tty.
    // We closed descriptor 0 already, so this will be the first one available
    int fd = open("/dev/null", O_RDWR);
    // We now re-open descriptors 1 and 2, in this order:
                           //O_RDWR
    fd = open(bbservLogFile, O_RDWR|O_CREAT|O_APPEND, 0644);
    dup(fd);
    
    // Puts itself into background.
    
    // Check and writes into the PID file bbserv.pid located in the current working directory.
    if (PIDFile != NULL) {
        char str[256];
        PIDFileDescriptor = open(PIDFile, O_RDWR|O_CREAT, 0644);
        if (PIDFileDescriptor < 0) {
            /* Can't open lockfile */
            exit(EXIT_FAILURE);
        }
        if (lockf(PIDFileDescriptor, F_TLOCK, 0) < 0) {
            /* Can't lock file */
            exit(EXIT_FAILURE);
        }
        /* Get current PID */
        sprintf(str, "%d\n", getpid());
        /* Write PID to lockfile */
        write(PIDFileDescriptor, str, strlen(str));
    }
}


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
    while ((n = tcpUtils -> readline(sd,req,ALEN-1)) != tcpUtils -> recv_nodata) {
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
