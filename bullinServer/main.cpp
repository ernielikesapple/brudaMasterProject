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
#include <queue>
#include <list>
#include <atomic>
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
pthread_mutex_t logger_mutex;
void logger(const char * msg) {
    pthread_mutex_lock(&logger_mutex);
    time_t tt = time(0);
    char* ts = ctime(&tt);
    ts[strlen(ts) - 1] = '\0';
    printf("%s: %s", ts, msg);
    fflush(stdout);
    pthread_mutex_unlock(&logger_mutex);
}

std::vector<pthread_t> preallocatedThreadsPool;
std::queue<int> tcpQueue;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
void* threadFunctionUsedByThreadsPool(void *arg);
std::atomic<bool> stopCondition(false);

//std::vector<pthread_t> currentBusyThreads;
//pthread_mutex_t currentBusyThreads_mutex = PTHREAD_MUTEX_INITIALIZER;

// server para
/*
 * Global variables, for monitoring.  Notice that it is global, and
 * thus visible in all the threads; which is good, because all the
 * client threads will want to write to it so that the monitor can
 * report meaningful information.
 */

//struct monitor_t {
//    pthread_mutex_t mutex;
//    unsigned int con_cur;
//    unsigned int con_count;
//    unsigned int con_time;
//    unsigned int bytecount;
//};

// Monitor statistics:
//monitor_t mon;

// all the singleTon
TcpUtils* tcpUtils = TcpUtils::newAInstance();
ConfigFileHandler* configFileHandler = ConfigFileHandler::newAInstance();

// 1.5 Startup and Reconfiguration
void loadConfigFile();
void daemonize();
void signalHandlers(int sig);

int startServer(int msock);
void closeServer();  // TODO: implement this function

/*
* The function that implements the monitor thread.
*/
//void* monitor (void* ignored);
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
                    exit(0);
                    break;
                case 'h':  // help menu
                    printhelpFunction();
                    exit(0);
                default:
                    
                    break;
            }
        } else {
            // TODO: use a string to append/take all the values from argv[optind] , and then after the while loop take everything into the config file
            std::cout << "handle non-switch argument are: " << argv[optind] << std::endl;
            // TODO: check if the non-switch argument are not meet the requirements of host:port, we need to exits
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

    signal(SIGQUIT,signalHandlers); // Installs appropriate signal handlers for all the signals.
    signal(SIGHUP,signalHandlers);
    
    // initializing
    
    pthread_mutex_init(&logger_mutex, 0);
    
    int port = 9000;   // port to listen to
    try { port = std::stoi(bp); } // The clients connect to our server on port bp.
    catch (std::invalid_argument const &e) { std::cout << "Bad input: std::invalid_argument thrown" << '\n'; }
    catch (std::out_of_range const &e) { std::cout << "Integer overflow: std::out_of_range thrown" << '\n'; }
    
    const int qlen = 32;     // incoming connections queue length

    // Note that the following are local variable, and thus not shared
    // between threads; especially important for ssock and client_addr.

    int msock;               // master and slave socket
    msock = passivesocket(port,qlen);

    if (msock < 0) {
     perror("passivesocket");
     exit(0); // this is where the program mostly has been terminated
    }

    printf("Server up and listening on port %d.\n", port);
    // Setting up the thread creation: for monitor thread
   // pthread_t tt;
//    pthread_attr_t ta;
//    pthread_attr_init(&ta);
//    pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);
    // Launch the monitor:
 //   pthread_create(&tt, &ta, monitor, NULL);
    
    
    flocks.mutex = PTHREAD_MUTEX_INITIALIZER;
    flocks.can_write = PTHREAD_COND_INITIALIZER;
    flocks.reads = 0;
    
    //  use preallocated threads. The number of threads to be preallocated is Tmax, so that Tmax is also a limit on concurrency.
    int preallocatThreadsNumber = 20;
    try { preallocatThreadsNumber = std::stoi(Tmax); } // The clients connect to our server on port bp.
    catch (std::invalid_argument const &e) { std::cout << "Bad input: std::invalid_argument thrown" << '\n'; }
    catch (std::out_of_range const &e) { std::cout << "Integer overflow: std::out_of_range thrown" << '\n'; }
       
    preallocatedThreadsPool.resize(preallocatThreadsNumber); // create a threadpoOl
    for(pthread_t &i : preallocatedThreadsPool) {
        pthread_create(&i, NULL, threadFunctionUsedByThreadsPool, NULL);
    }
    
    startServer(msock);
    
    return 0;
}


int startServer(int msock) {
    int ssock;
    struct sockaddr_in client_addr; // the address of the client...
    unsigned int client_addr_len = sizeof(client_addr); // ... and its length
    
    while (!stopCondition) {
        // Accept connection:
        ssock = ::accept((int)msock, (struct sockaddr*)&client_addr, &client_addr_len);   //  the return value is a socket
    
        if (ssock < 0) {
             if (errno == EINTR) continue;
             perror("accept");
             return 0;
        } else {
            //  incoming client will wait in the TCP queue until something becomes available
            pthread_mutex_lock(&mutex); // one thread mess with the queue at one time
            tcpQueue.push(ssock);
            pthread_cond_signal(&condition_var);
            pthread_mutex_unlock(&mutex);
        }
        
     // main thread continues with the loop...
    }
    
    return 1;
    
}

void* threadFunctionUsedByThreadsPool(void *arg) {
    int newClient = 0;
    pthread_mutex_lock(&mutex);
    while (!stopCondition) {
           // wait for a task to be queued
       while (tcpQueue.empty() && !stopCondition) {
           pthread_cond_wait(&condition_var, &mutex); // wait for the signal from other thread to deal with client otherwise sleep
       }
       if (stopCondition == false) {
           newClient = tcpQueue.front();
           tcpQueue.pop();
           // exit lock while operating on a task
           pthread_mutex_unlock(&mutex);
           if (newClient) {
               do_client(newClient);
           }
           // re-acquire the lock
           pthread_mutex_lock(&mutex);
       }
    }
   // release the lock before exiting the function
    pthread_mutex_unlock(&mutex);
   return NULL;
}


/*
 * Repeatedly receives requests from the client and responds to them.
 * If the received request is an end of file or "quit", terminates the
 * connection.  Note that an empty request also terminates the
 * connection.  Same as for the purely iterative or the multi-process
 * server.
 */
void* do_client (int sd) {
    
//    pthread_mutex_lock(&currentBusyThreads_mutex);
//    currentBusyThreads.push_back(pthread_self());
//    pthread_mutex_unlock(&currentBusyThreads_mutex);

    char req[MAX_LEN];   // the content sent by the client
    // const char* ack = "ACK: ";
    int n;
    // make sure req is null-terminated...
    req[MAX_LEN-1] = '\0';
    
    struct User user;
    user.username = "nobody";
    
//    time_t start = time(0); 
    printf("Incoming client...\n");
    // monitor code begins
//    pthread_mutex_lock(&mon.mutex);                    //  when u define a mutex, you will define a critical region
//    mon.con_cur++;
//    pthread_mutex_unlock(&mon.mutex);
    // monitor code ends

    // Loop while the client has something to say...
    while ((n = readline(sd,req,MAX_LEN-1)) != recv_nodata && !stopCondition) {
       // std::cout << " ===do_client====readline====="  << std::endl;

        std::string usage = "Commands avaiable: \n";
        std::string greetingsUsage = "Greeting \n";
        std::string userUsage = "USER name \n";
        std::string readUsage = "READ message-number \n";
        std::string writeUsage = "WRITE message \n";
        std::string replaceUsage = "REPLACE message-number/message \n";
        std::string quitUsage = "QUIT text \n";
        
        usage = "Commands avaiable: \n"  +  greetingsUsage + userUsage + readUsage + writeUsage + replaceUsage + quitUsage;
        std::string ans = "unrecognized commands \n\n" + usage;
        
        // If we talk to telnet, we get \r\n at the end of the line
        // instead of just \n, so we take care of the possible \r:
        if ( n > 1 && req[n-1] == '\r' )
            req[n-1] = '\0';
        
        if ( strncasecmp(req,"QUIT",strlen("QUIT")) == 0 ) {  // The strncasecmp() function is similar, except it only compares the first n bytes of req.
            
            send(sd,"BYE see you next time\r\n", strlen("BYE see you next time\r\n"),0);
            
            printf("Received quit, sending EOF.\n");
            shutdown(sd,1);
            close(sd);
            std::cout << " ======do_client==  用自己线程挂你比就能返回正常值=="<<  close(sd) << "===="<<        shutdown(sd,1) << std::endl;


            printf("Outgoing client...\n");
            // monitor code begins
//            pthread_mutex_lock(&mon.mutex);
//            mon.con_cur--;
//            mon.con_count++;
//            mon.con_time += time(0) - start;
//            pthread_mutex_unlock(&mon.mutex);
            // monitor code endsb
        
            
            
            return NULL;
        }
        
        else if (strncasecmp(req,"Greeting",strlen("Greeting")) == 0 ) {
            std::string greetingResponse = "0.0 greeting \n";
            
            std::string usage = "Commands avaiable: \n";
            std::string greetingsUsage = "Greeting \n";
            std::string userUsage = "USER name \n";
            std::string readUsage = "READ message-number \n";
            std::string writeUsage = "WRITE message \n";
            std::string replaceUsage = "REPLACE message-number/message \n";
            std::string quitUsage = "QUIT text \n";
            
            usage = "Commands avaiable: \n"  +  greetingsUsage + userUsage + readUsage + writeUsage + replaceUsage + quitUsage;
            
            ans = greetingResponse + usage;
        }
        
        else if (strncasecmp(req,"USER",strlen("USER")) == 0 ) {
            int nextArgIndex = next_arg(req,' ');
            
            if (nextArgIndex == -1 ) {
                // TODO: response texts not in full length?
                ans = "USER command requires a username, Format: 'USER name'";
            }
            else {
                std::string str(&req[nextArgIndex]);  // &req[nextArgIndex] refers to whatever sent by the user after the USER commands, req[nextArgIndex] is the first letter of the word(which is after the USER commands) sent by user,
                size_t found = str.find("/");
                if (found != std::string::npos) {
                    ans = "USER's argument name is a string not containing the character /.";
                } else {
                    std::string username(&req[nextArgIndex]);
                    user.username = username;
                    ans = "HELLO " + username + " welcome to bbserv";
                }
            }
        }
        
        else if (strncasecmp(req,"READ",strlen("READ")) == 0 ) {
            int nextArgIndex = next_arg(req,' ');
            if (nextArgIndex == -1) {
                ans = "READ command requires a message number, Format: 'READ message-number'";
            }
            else {
                std::string messageNumber(&req[nextArgIndex]);
                ans = bbfileReader(bbfile, messageNumber);
            }
        }
        
        else if (strncasecmp(req,"WRITE",strlen("WRITE")) == 0 ) {
            int nextArgIndex = next_arg(req,' ');
            if (nextArgIndex == -1) {
                ans = "WRITE command requires a message, Format: 'WRITE message'";
            }
            else {
                std::string message(&req[nextArgIndex]);
                ans = bbfileWritter(bbfile, user.username, message);
            }
        }
        
        else if (strncasecmp(req,"REPLACE",strlen("REPLACE")) == 0 ) {
            int nextArgIndex = next_arg(req,' ');
            if (nextArgIndex == -1) {
                ans = "REPLACE command requires a message, Format: 'REPLACE message-number/message'";
            }
            else {
                std::string messageNumberPlusMessage(&req[nextArgIndex]);
                size_t foundSlash= messageNumberPlusMessage.find_first_of('/');
                if (foundSlash != std::string::npos) {
                    std::string messageNumber = messageNumberPlusMessage.substr(0, foundSlash);
                    std::string newMessage = messageNumberPlusMessage.substr(foundSlash + 1, messageNumberPlusMessage.length()); // + 1 to skip the orignal '/'
                    if (messageNumber.length()>0 && newMessage.length()>0) {
                        ans = bbfileReplacer(bbfile, messageNumber, newMessage, user.username);  // replace file
                    } else {
                        ans = "REPLACE command requires a proper format to continue, Format: 'REPLACE message-number/message'";
                    }
                } else {
                    ans = "REPLACE command requires a proper format to continue, Format: 'REPLACE message-number/message'";
                }
            }
            
        }
        
        
        // monitor code begins
//        pthread_mutex_lock(&mon.mutex);
//        mon.bytecount += n;
//        pthread_mutex_unlock(&mon.mutex);
        // monitor code ends
         

        send(sd,&ans[0],ans.size(),0);
        send(sd,"\r\n",2,0);        // telnet expects \r\n
        // ans.clear();
        // send(sd,"\n",1,0);
        /*
        send(sd,ack,strlen(ack),0);
        send(sd,req,strlen(req),0);
        send(sd,"\n",1,0);
         */
        
        if (stopCondition) {
            shutdown(sd,1);
            close(sd);
            break;
        }
        
    } // while ends
    
    
    
    
    // read 0 bytes = EOF:
    printf("Connection closed by client.\n");
    shutdown(sd,1);
    close(sd);
    printf("Outgoing client...\n");
    
    
    // monitor code begins
//    pthread_mutex_lock(&mon.mutex);
//    mon.con_cur--;
//    mon.con_count++;
//    mon.con_time += time(0) - start;
//    pthread_mutex_unlock(&mon.mutex);
    // monitor code ends
    
    return NULL;
}

void signalHandlers(int sig) { //TODO: Handle all the signals
    // deinitialize everything and destruct everything
    
    // signal all threads to exit after they finish their current work item
    pthread_mutex_lock(&mutex);
        stopCondition = true;
        pthread_cond_broadcast(&condition_var); // notify all threads
    pthread_mutex_unlock(&mutex);
    
    // Closes all the master sockets  // TODO: can't close master socket, since it's blocking on the
//     close(currentMasterSocket);
//     shutdown(currentMasterSocket,1);
    
    // closes all the connections to all the clients   // TODO: can't implement this
    
    
//    for (auto& t : currentBusyThreads) {
//            std::cout << " ======signalHandlers==currentBusyThreads 1==" << t << std::endl;
//        pthread_cancel(t);
//             pthread_join(t, nullptr);
//        //    pthread_kill(t, 9);
////            std::cout << " ======signalHandlers==2.2=currentBusyThreads=" << t << std::endl;
//    }
//    currentBusyThreads.clear();
    
    
    // wait for all threasd to exit, terminates all the preallocated threads
    
    pthread_mutex_lock(&mutex);
        stopCondition = false;
    pthread_mutex_unlock(&mutex);


    if (sig == SIGQUIT) { // Quit the daemon
        
        /* Unlock and close lockfile */
        if (PIDFileDescriptor != -1) {
            lockf(PIDFileDescriptor, F_ULOCK, 0);
            close(PIDFileDescriptor);
        }
        /* Try to delete lockfile */
        if (PIDFile != NULL) {
            remove(PIDFile);
        }
                
        /* Reset signal handling to default behavior */
     //   signal(SIGINT, SIG_DFL);
    } else if (sig == SIGHUP) {
            
        loadConfigFile(); // reload the config file
        
        
        int port = 9000;   // port to listen to
        try { port = std::stoi(bp); } // The clients connect to our server on port bp.
        catch (std::invalid_argument const &e) { std::cout << "Bad input: std::invalid_argument thrown" << '\n'; }
        catch (std::out_of_range const &e) { std::cout << "Integer overflow: std::out_of_range thrown" << '\n'; }
        
        const int qlen = 32;     // incoming connections queue length

        // Note that the following are local variable, and thus not shared
        // between threads; especially important for ssock and client_addr.

        int msock;               // master and slave socket
        msock = passivesocket(port,qlen);
        if (msock < 0) {
         perror("passivesocket");
        }
         printf("Server up and listening on port %d.\n", port);
        
        
        //  use preallocated threads. The number of threads to be preallocated is Tmax, so that Tmax is also a limit on concurrency.
        int preallocatThreadsNumber = 20;
        try { preallocatThreadsNumber = std::stoi(Tmax); } // The clients connect to our server on port bp.
        catch (std::invalid_argument const &e) { std::cout << "Bad input: std::invalid_argument thrown" << '\n'; }
        catch (std::out_of_range const &e) { std::cout << "Integer overflow: std::out_of_range thrown" << '\n'; }
           
        preallocatedThreadsPool.resize(preallocatThreadsNumber); // create a threadpoOl
        for(pthread_t &i : preallocatedThreadsPool) {
            pthread_create(&i, NULL, threadFunctionUsedByThreadsPool, NULL);
        }

         startServer(msock);
         
        
    } else if (sig == SIGCHLD) {
        // TODO: handle SIGCHLD
    }
    signal(SIGINT, SIG_DFL);
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


//void* monitor (void* ignored) {
//    const int wakeup_interval = 120; // 2 minutes
//    int connections;
//    
//    while (1) {
//        sleep(wakeup_interval);
//        pthread_mutex_lock(&mon.mutex);
//        // time_t now = time(0);
//        if (mon.con_count == 0)
//            connections = 1;
//        else
//            connections = mon.con_count;
//        /*
//        printf("MON: %s\n", ctime(&now));
//        printf("MON: currently serving %d clients\n", mon.con_cur);
//        printf("MON: average connection time is %d seconds.\n",
//               (int)((float)mon.con_time/(float)connections));
//        printf("MON: transferred %d bytes per connection on average.\n",
//               (int)((float)mon.bytecount/(float)connections));
//        printf("MON: (end of information)\n");
//         */
//        
//        pthread_mutex_unlock(&mon.mutex);
//    }
//    return 0;
//}
