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
#include <vector>
#include <atomic>
#include "TcpUtils.hpp"
#include "ConfigFileHandler.hpp"

char const* app_name = "bbserv";
std::string configFileName = "bbserv.conf";
std::string Tmax = "20";   // keyValue in the config file: THMAX   // use stoi(), change int to string
std::string bp = "9000";   // keyValue in the config file: BBPORT , port number for client server communication
std::string sp = "10000";  // keyValue in the config file: SYNCPORT,  port number for server server communication
std::string bbfile = "bbfile";   // keyValue in the config file: BBFILE               // config fileName mandatory data if it's empty then refuse to start the server,
std::string peers = "";    // keyValue in the config file: PEERS
std::vector<std::pair<std::string, int> > peersHostNPortVector;
bool d = true;   // keyValue in the config file: DAEMON
bool D = false; // keyValue in the config file: DEBUG

char const* PIDFile = "bbserv.pid"; // the lock file, to lock the critical region so that only one daemon is runnning at a time
static int PIDFileDescriptor = -1;

char const* bbservLogFile = "bbserv.log"; // redirct all the output to this file when under daemon mode

// log facility
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

// thread pool related
std::vector<pthread_t> preallocatedThreadsPool;
std::queue<int> tcpQueue;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
void* threadFunctionUsedByThreadsPool(void *arg);
std::atomic<bool> stopCondition(false);

std::vector<pthread_t> currentBusyThreads;
pthread_mutex_t currentBusyThreads_mutex = PTHREAD_MUTEX_INITIALIZER;

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

pthread_t clientServerThread;  // communicate with client
void*  startServer(void *arg);
void closeServer();  // TODO: implement this function
int currentMasterSocket;
/*
* The function that implements the monitor thread.
*/
//void* monitor (void* ignored);
void* do_client (int sd);

pthread_t slaveServerThread;  // server to server communication
void*  startUpSlaveServer(void *arg);
void* do_syncronazation (int sd);

int connectToAllthePeersForSyncronazation (std::string operationMethod, std::string messageNumber, std::string message, std::string poster);  // return 0 means twoPhaseCommit fail, 1 means two phase all success, TODO: add function parameter accordingly


int main(int argc, char** argv) {
    
    loadConfigFile(); // load the value from config file into all the global variables
    
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
    // handle switch arguments
    while ( (option = getopt(argc, argv, "b:c:T:t:p:s:fdh")) != -1 ) {  // for each option...
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
            case 'T': { // overide Tmax number, preallocated threads
                std::string switchValue = optarg;
                std::string::const_iterator itForTRaw = switchValue.begin();
                while (itForTRaw != switchValue.end() && std::isdigit(*itForTRaw)) {++itForTRaw;}
                if(!switchValue.empty() && itForTRaw == switchValue.end()) { // check all the ports are digits
                    configFileHandler -> configFileModifier(configFileName,"THMAX",optarg);
                    Tmax = optarg;
                    break;
                } else {
                   std::cerr << "switch argument T are not in the correct format and the thread number should be all in digits, format :  '-T threadsNumber'" << std::endl;
                   std::exit(-1);
                }
                break;
            }
            case 't': { // overide Tmax number, preallocated threads
                std::string switchValue = optarg;
                std::string::const_iterator itForTRaw = switchValue.begin();
                while (itForTRaw != switchValue.end() && std::isdigit(*itForTRaw)) {++itForTRaw;}
                if(!switchValue.empty() && itForTRaw == switchValue.end()) { // check all the ports are digits
                    configFileHandler -> configFileModifier(configFileName,"THMAX",optarg);
                    Tmax = optarg;
                    break;
                } else {
                   std::cerr << "switch argument t are not in the correct format and the thread number should be all in digits, format :  '-t threadsNumber'" << std::endl;
                   std::exit(-1);
                }
                break;
            }
            case 'p': { // client server port number
                std::string switchValue = optarg;
                std::string::const_iterator itForPortRaw = switchValue.begin();
                while (itForPortRaw != switchValue.end() && std::isdigit(*itForPortRaw)) {++itForPortRaw;}
                if(!switchValue.empty() && itForPortRaw == switchValue.end()) { // check all the ports are digits
                    configFileHandler -> configFileModifier(configFileName,"BBPORT",optarg);
                    bp = optarg;
                    break;
                } else {
                   std::cerr << "switch argument p are not in the correct format and the port number should be all in digits, format :  '-p portNumber'" << std::endl;
                   std::exit(-1);
                }
                break;
            }
            case 's': {// server port number
                std::string switchValue = optarg;
                std::string::const_iterator itForPortRaw = switchValue.begin();
                while (itForPortRaw != switchValue.end() && std::isdigit(*itForPortRaw)) {++itForPortRaw;}
                if(!switchValue.empty() && itForPortRaw == switchValue.end()) { // check all the ports are digits
                    configFileHandler -> configFileModifier(configFileName,"SYNCPORT",optarg);
                    sp = optarg;
                    break;
                } else {
                   std::cerr << "switch argument s are not in the correct format and the port number should be all in digits, format :  '-s portNumber'" << std::endl;
                   std::exit(-1);
                }
                break;
            }
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
    }
 
            
    // handle non switch arguments
    std::string peersString = ""; // use a string to append/take all the values from argv[i] , and then after the while loop take everything into the config file
    argc -= optind - 1;
    argv += optind - 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "") != 0) {
            std::string peerStringRaw = argv[i];
            size_t found  = peerStringRaw.find(":");
            peersHostNPortVector.clear(); // clean the value from last time use the latest value from command line
            // check if the non-switch argument are not meet the requirements of host:port, we need to exits
            if (found != std::string::npos) { // check the host and port are in the correct format
                std::string domainRaw = peerStringRaw.substr(0, found); // check if host is valide
                struct hostent *h;
                if ((h=gethostbyname(domainRaw.c_str())) == NULL) {
                    std::cerr << "non-switch argument are not in the correct format and the host supplied is not available, format :  'host:port host:post host:post...' " << std::endl;
                    std::exit(-1);
                }
                // check the port is valid
                std::string portRaw = peerStringRaw.substr(found+1, peerStringRaw.length());
                std::string::const_iterator itForPortRaw = portRaw.begin();
                while (itForPortRaw != portRaw.end() && std::isdigit(*itForPortRaw)) {++itForPortRaw;}
                if(!portRaw.empty() && itForPortRaw == portRaw.end()) { // check all the ports are digits
                    peersHostNPortVector.push_back(make_pair(domainRaw,stoi(portRaw))); // update the vector in the global variable for later uses
                    // use a string to append/take all the values from argv[i] , and then after the while loop take everything into the config file
                    peersString = peersString + argv[i] + " "; // update peerString for recoring in the config file
                } else {
                    std::cerr << "non-switch argument are not in the correct format and the port number should be all in digits, format :  'host:port host:post host:post...'" << std::endl;
                    std::exit(-1);
                }
            } else {
                std::cerr << "non-switch argument are not in the correct format, format :  'host:port host:post host:post...'" << std::endl;
                exit(0);
            }
        }
    }
    
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
    
    
    
    pthread_create(&clientServerThread, NULL, startServer, NULL);
    pthread_create(&slaveServerThread, NULL, startUpSlaveServer, NULL);

    
    
    /* Wait till threads are complete before main continues. Unless we  */
    /* wait we run the risk of executing an exit which will terminate   */
    /* the process and all threads before the threads have completed.   */
    pthread_join(clientServerThread, NULL);
    pthread_join(slaveServerThread, NULL);
    
    return 0;
}

void*  startServer(void *arg) {
    
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
    currentMasterSocket = msock;
    
    //  use preallocated threads. The number of threads to be preallocated is Tmax, so that Tmax is also a limit on concurrency.
    int preallocatThreadsNumber = 20;
    try { preallocatThreadsNumber = std::stoi(Tmax); } // The clients connect to our server on port bp.
    catch (std::invalid_argument const &e) { std::cout << "Bad input: std::invalid_argument thrown" << '\n'; }
    catch (std::out_of_range const &e) { std::cout << "Integer overflow: std::out_of_range thrown" << '\n'; }
       
    preallocatedThreadsPool.resize(preallocatThreadsNumber); // create a threadpoOl
    for(pthread_t &i : preallocatedThreadsPool) {
        pthread_create(&i, NULL, threadFunctionUsedByThreadsPool, NULL);
    }
    
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
    
    return NULL;
    
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
    
    pthread_mutex_lock(&currentBusyThreads_mutex);
    currentBusyThreads.push_back(pthread_self());
    pthread_mutex_unlock(&currentBusyThreads_mutex);

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
        
        if ( strncasecmp(req,"",strlen("")) == 0 ) {  // handle return
            ans = "";
        }
        
        if ( strncasecmp(req,"QUIT",strlen("QUIT")) == 0 ) {  // The strncasecmp() function is similar, except it only compares the first n bytes of req.
            
            send(sd,"BYE see you next time\r\n", strlen("BYE see you next time\r\n"),0);
            
            printf("Received quit, sending EOF.\n");
            shutdown(sd,1);
            close(sd);
            
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
                
                // logic for peers Synchronization

                if (peersHostNPortVector.size() > 0) { // only wrote success, and there are peers , we will try to do the synchronizaiton,  after the sync process, if two phase commit fail, then update the ans message to the client, other wise remains the same
                     // TODO: add logic for synchronazition
                    ans = bbfileWritter(bbfile, user.username, message);
                    size_t positionOfCurrentMessageNumber = ans.find_first_of(' ');
                    std::string currentMessageNumber = ans.substr(positionOfCurrentMessageNumber + 1, (ans.length() - positionOfCurrentMessageNumber));
                    
                    int result = connectToAllthePeersForSyncronazation("write", currentMessageNumber, message, user.username);  // pass necessary message to
                    if (result == 0) {  //
                        bbfileDeleterUsedWhenSynChronization(bbfile, currentMessageNumber ,user.username, message); // abandon the written previous messages
                        ans = "ERROR WRITE can not sync all the message from client, due to the peers server synchronization error";
                    } else { // peers synchronization success, we perform the operation on the master server too
                        // ans = bbfileWritterUsedWhenSynChronization(bbfile, currentMessageNumber, user.username, message);
                    }
                } else { // there is no peers specified in the config file or command line, we only perform one client one server operation
                    ans = bbfileWritter(bbfile, user.username, message);
                }
                
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
                        
                        if (peersHostNPortVector.size() > 0) { // only wrote success, and there are peers , we will try to do the synchronizaiton,  after the sync process, if two phase commit fail, then update the ans message to the client, other wise remains the same
                             // TODO: add logic for synchronazition
                            int result = connectToAllthePeersForSyncronazation("replace",messageNumber, newMessage, user.username);  // pass necessary message to
                            if (result == 0) {  //
                                ans = "ERROR WRITE can not sync all the message from client, due to the peers server synchronization error";
                            } else { // peers synchronization success, we perform the operation on the master server too
                                ans = bbfileReplacer(bbfile, messageNumber, newMessage, user.username);  // replace file
                            }
                        } else { // there is no peers specified in the config file or command line, we only perform one client one server operation
                                ans = bbfileReplacer(bbfile, messageNumber, newMessage, user.username);  // replace file
                        }
                        
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
    
    pthread_cancel(clientServerThread);
    pthread_cancel(slaveServerThread);
    
    
    
    // TODO: wake up accept blokcing call using pipe() https://stackoverflow.com/questions/2486335/wake-up-thread-blocked-on-accept-call
    
   
//     pthread_kill(clientServerThread, 9);
//     pthread_kill(slaveServerThread, 9);
//
    // closes all the connections to all the clients   // TODO: can't implement this
    
    
    for (auto& t : currentBusyThreads) { // clean up the thread which is blocked on read()
        pthread_cancel(t);
        pthread_join(t, nullptr);
    }
    currentBusyThreads.clear();
    
    
    // Closes all the master sockets  // TODO: can't close master socket, since it's blocking on the
     close(currentMasterSocket);   // why after cancel still can't close it?
     shutdown(currentMasterSocket,1);
     

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
        
        pthread_create(&clientServerThread, NULL, startServer, NULL);
        pthread_create(&slaveServerThread, NULL, startUpSlaveServer, NULL);
        
        pthread_join(clientServerThread, NULL);
        pthread_join(slaveServerThread, NULL);
        
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
        
        configFileHandler ->configFileValueGetter("PEERS", peers);
        peersHostNPortVector.clear(); // clean the value from last time use the latest value from config file
        if (strcmp(peers.c_str(), "") != 0) {  // check the peers are all in the valide format and to verify the value inside the server function make sure it has value
            std::string::iterator itForPeers = peers.begin();
            size_t theBeginningIndexOfAHostNPort = 0;
            while (itForPeers != peers.end()) {
                const char currentChar = *itForPeers;

                auto index = std::distance(peers.begin(), itForPeers); // get the current index of the iterator

                const char nextChar = *(itForPeers+1);

                if ((currentChar == ' '&& nextChar != ' ') || (itForPeers - peers.begin() == (peers.length()-1))) { // skip all the space between host:port host port, or there is only one host port
                    std::string peerStringRaw = peers.substr(theBeginningIndexOfAHostNPort, (index-theBeginningIndexOfAHostNPort + 1)); // notice the send parameter means the length
                    if(peerStringRaw.find_first_not_of(' ') != std::string::npos) { // remove the white space before first host:post                        
                        theBeginningIndexOfAHostNPort = index + 1;  // the index after the last host:port + space
                        
                        peerStringRaw.erase(std::remove_if(peerStringRaw.begin(), peerStringRaw.end(), ::isspace),peerStringRaw.end()); // remove the white space
                        
                        size_t found  = peerStringRaw.find(":");
                        if (found != std::string::npos) { // check the host and port are in the correct format
                            std::string domainRaw = peerStringRaw.substr(0, found); // check if host is valide
                            struct hostent *h;
                            if ((h=gethostbyname(domainRaw.c_str())) == NULL) {

                                std::cerr << "warning: peers in the config file are not in the correct format and the host supplied is not available, format :  'peers=host:port host:post host:post...' " << std::endl;
                                break;
                            }
                            // check the port is valid
                            std::string portRaw = peerStringRaw.substr(found+1, peerStringRaw.length());
                            std::string::const_iterator itForPortRaw = portRaw.begin();
                            while (itForPortRaw != portRaw.end() && std::isdigit(*itForPortRaw)) ++itForPortRaw;
                            if(!portRaw.empty() && itForPortRaw == portRaw.end()) {
                                peersHostNPortVector.push_back(make_pair(domainRaw,stoi(portRaw)));
                            } else {
                                std::cerr << "warning: peers in the config file are not in the correct format and the port number should be all in digits, format :  'peers=host:port host:post host:post...' " << std::endl;
                            }
                            
                        } else {
                            std::cerr << "warning: peers in the config file are not in the correct format, format :  'peers=host:port host:post host:post...' " << std::endl;
                        }
                    }
                }
                itForPeers++;
            }
        }
        
        
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


void*  startUpSlaveServer(void *arg) {  // receive the message from master server
    
    int port = 9000;   // port to listen to
    try { port = std::stoi(sp); } // The master server connect to our back up server on port sp.
    catch (std::invalid_argument const &e) { std::cout << "Bad input: std::invalid_argument thrown" << '\n'; }
    catch (std::out_of_range const &e) { std::cout << "Integer overflow: std::out_of_range thrown" << '\n'; }
    
    const int qlen = 32;     // incoming connections queue length
      
      // Note that the following are local variable, and thus not shared
      // between threads; especially important for ssock and client_addr.
      
      long int msock, ssock;               // master and slave socket
    
      struct sockaddr_in client_addr; // the address of the client...
      unsigned int client_addr_len = sizeof(client_addr); // ... and its length
      
      msock = passivesocket(port,qlen);
      if (msock < 0) {
          perror("passivesocket");
          return NULL;
      }
      printf("back Server up and listening on port %d.\n", port);

      // Setting up the thread creation:
      pthread_t tt;                       // thread id
      pthread_attr_t ta;                  // thread attribute, need to initialize it
      pthread_attr_init(&ta);
      pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);  // usually the thread is attached with parents, that means , a child thread is never going to terminate until the parent wait for(join) it, , after it detached from the parent , the parent can not wait for it(join it), if parent exit, the child will continue exit
      
      while (!stopCondition) {
          // Accept connection:
          ssock = accept((int)msock, (struct sockaddr*)&client_addr, &client_addr_len);

          if (ssock < 0) {
              if (errno == EINTR) continue;
              perror("accept");
              return NULL;
          }
          
          // Create thread instead of fork:
          if ( pthread_create(&tt, NULL, (void* (*) (void*))do_syncronazation, (void*)ssock) != 0 ) {         // pthread_create takes three para, 1st address of thread id, 2nd, address of thread attribute, each thread also need a main function,function pointer,  4th para is the para for the function, (void*) is casted as a void pointer
              perror("pthread_create");
              return NULL;
          }
          // main thread continues with the loop...
      }
    
    return NULL;
    
}

void* do_syncronazation (int sd) {
    while (!stopCondition) {
        // TODO: logic for synchronazition
        char precommitMessage[256];  // precommitMessage from master server
        bzero(precommitMessage, 256);
        int precommitMessageStatus = (int)read(sd, precommitMessage, sizeof(precommitMessage)); // the receive status of first message
        if (D) {
            char msg[256];
            snprintf(msg, 256, "Precommit phase: slave server receives:  %s from master server \n", precommitMessage);
            logger(msg);
        }
        
        try {
            if (precommitMessageStatus != 0) {
                send(sd, "READY", 256, 0);
                if (D) {
                    std::string logMessage = "Precommit phase: slave server sends 'Ready' to master Server \n";
                    logger(&logMessage[0]);
                }
            }
        } catch(const std::system_error& e) {
            send(sd, "ABORT", 256, 0);
            if (D) {
                std::string logMessage = "Precommit phase: slave server sends 'ABORT' to master Server \n";
                logger(&logMessage[0]);
            }
            std::cout << "Caught system_error with code " << e.code()
                      << " meaning " << e.what() << '\n';
            return NULL;
        }
        
        
        char commitMessages[256];
        read(sd, commitMessages, sizeof(commitMessages));  // read receiveCommitStatus
        if (D) {
           char msg[256];
           snprintf(msg, 256, "Commit phase: slave server receives commit messages:  %s from master server \n", commitMessages);
           logger(msg);
        }

        if ((strcmp(commitMessages, "ABORT")) == 0) {
            return NULL;
        }
        else if ((strcmp(commitMessages, "ABORT")) != 0) {

            int i;
            std::string commitMessagesStr = "";
            for (i = 0; i < sizeof(commitMessages); i++) {  // convert char[] to string
                commitMessagesStr = commitMessagesStr + commitMessages[i];
            }
            
            size_t foundSlash= commitMessagesStr.find_first_of('/');
            
            std::string operationMethod = commitMessagesStr.substr(0, foundSlash);
            std::string messageNumberNPosterNMessage = commitMessagesStr.substr(foundSlash + 1, (commitMessagesStr.length() - operationMethod.length()));
            
            
            size_t foundSlash1 = messageNumberNPosterNMessage.find_first_of('/');
            std::string messageNumber = messageNumberNPosterNMessage.substr(0, foundSlash1);
            
            std::string posterNMessage = messageNumberNPosterNMessage.substr(foundSlash1 + 1, (messageNumberNPosterNMessage.length() - messageNumber.length())); // + 1 to skip the orignal '/'
            size_t foundSlash2= posterNMessage.find_first_of('/');
            std::string poster = posterNMessage.substr(0, foundSlash2);
            std::string message = posterNMessage.substr(foundSlash2 + 1, (posterNMessage.length() - poster.length()));
            
            std::string operationResult = "";  // store commit messages
            if (operationMethod == "write") { // means it's a write operation
                operationResult = bbfileWritterUsedWhenSynChronization(bbfile, messageNumber ,poster, message);
            } else { // means it's a replace operation
                operationResult = bbfileReplacer(bbfile, messageNumber, message, poster);  // replace file
            }
            
            if (strncasecmp(operationResult.c_str(),"WROTE",strlen("WROTE")) == 0 ) { // operation sucess
                send(sd, "commit Sucess", 256, 0);
            } else {
                send(sd, "ABORT", 256, 0);
            }
                
            char commitMessagesFeedback[256];
            read(sd, commitMessagesFeedback, sizeof(commitMessagesFeedback));  // read all the commit messages if ok return NULL, if not abondan the previous write/replace operation
            if (D) {
               char msg[256];
               snprintf(msg, 256, "Commit phase: slave server receives commit messages Response :  %s from master server \n", commitMessagesFeedback);
               logger(msg);
            }

            if ((strcmp(commitMessagesFeedback, "ABORT")) == 0) {
                bbfileDeleterUsedWhenSynChronization(bbfile, messageNumber ,poster, message); // abandon the written previous messages
                return NULL;
            }
            else if ((strcmp(commitMessagesFeedback, "ABORT")) != 0) {
                return NULL;
            }
        }
        
        return NULL;
    }
    
    close(sd);
    
    return NULL;
}


int connectToAllthePeersForSyncronazation (std::string operationMethod, std::string messageNumber, std::string message, std::string poster) {  //
    
    std::vector<std::pair<int, bool> > peersSocketDescriptorNOnlinestatus;
    
    for (auto& it : peersHostNPortVector) { // clean up the thread which is blocked on read()
        std::string host = it.first;
        std::string s = std::to_string(it.second);
        char const *port = s.c_str();
        
        int sd = connectbyport(host.c_str(),port);   // host and port number
        if (sd == err_host) {
           fprintf(stderr, "Cannot find host %s.\n", host.c_str());
           return 0;
        }
        if (sd < 0) {
           perror("connectbyport");
           return 0;
        }
        
        peersSocketDescriptorNOnlinestatus.push_back(std::make_pair(sd,true));
    }
    
    while (!stopCondition) {
        
        
        for (auto& it : peersSocketDescriptorNOnlinestatus) {  // send pre-commit message to each online peer
            if (it.second == true) { // means the peer server is online
                send(it.first, "VOTE", 256, 0);
                if (D) {
                    std::string logMessage = "Precommit phase: master server sends VOTE, as pre-commit message to collect all the peers slave server status \n";
                    logger(&logMessage[0]);
                }
                it.second = false; // set back to init value
            }
        }
        
        
        //Recieve status reply from the backend servers
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        
        char preCommitResponse[256];
        int onlinePeersServerCount = 0;
        int abortedCount = 0;
        for (auto& it : peersSocketDescriptorNOnlinestatus) {  // send pre-commit message to each online peer
            if (setsockopt(it.first, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
                std::cout << "Timeout error" << std::endl;
                return 0;
            }
            while (recv(it.first, preCommitResponse, sizeof(preCommitResponse), 0) > 0) {
                if (D) {
                    char msg[256];
                    snprintf(msg, 256, "Precommit phase: master server receives the pre-commit message responses: %s from the peers slave server status \n", preCommitResponse);
                    logger(msg);
                }
                
                if (strcmp(preCommitResponse, "ABORT") == 0) {
                    abortedCount++;
                }

                onlinePeersServerCount++;
                it.second = true;
            }
        }
        
        if (abortedCount > 0) { // there is at least one server is aborted
            char arr[] = "ABORT";
            for (auto& it : peersSocketDescriptorNOnlinestatus) {  // broadcast abort messages to all the peers
                send(it.first, arr, sizeof(arr), 0);
                if (D) {
                    std::string logMessage = "Precommit phase: master server broadcast the ABORT message to all the peers slave server, since there was one server is down  \n";
                    logger(&logMessage[0]);
                }
            }
            return 0;
        }
        else {

            if (onlinePeersServerCount == peersSocketDescriptorNOnlinestatus.size()) {
                
                for (auto& it : peersSocketDescriptorNOnlinestatus) {  // send pre-commit message to each online peer
                    if (it.second == true) { // means the peer server is online
                        
                        std::string commitMessages = "";
                        
                        if (operationMethod == "write") {
                           commitMessages = "write/" + messageNumber + '/' +  poster + '/' + message;
                        } else {
                           commitMessages = "replace/" + messageNumber + '/' +  poster + '/' + message;
                        }
                        
                        send(it.first, &commitMessages[0], commitMessages.size(), 0);
                        if (D) {
                            std::string logMessage = "Commit phase: master server sends commit and necessary data: " + commitMessages + " to all the peers slave server \n";
                            logger(&logMessage[0]);
                        }
                        
                        it.second = false; // set back to init value
                    }
                }
                
                struct timeval timeout;
                timeout.tv_sec = 6.5;
                timeout.tv_usec = 0;
                
                char secondCommitResponse[256];
                int successCommitedPeersServerCount = 0;
                int abortedCount = 0;
                for (auto& it : peersSocketDescriptorNOnlinestatus) {  // send pre-commit message to each online peer
                    if (setsockopt(it.first, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
                        std::cout << "Timeout error" << std::endl;
                        return 0;
                    }
                    while (recv(it.first, secondCommitResponse, sizeof(secondCommitResponse), 0) > 0) {
                        if (D) {
                            char msg[256];
                            snprintf(msg, 256, "Commit phase: master server receives commit status : %s from the peers slave server \n", secondCommitResponse);
                            logger(msg);
                        }
                        if (strcmp(secondCommitResponse, "ABORT") == 0) {
                            abortedCount++;
                        }
                        successCommitedPeersServerCount++;
                        it.second = true;
                    }
                }
                                
                if (abortedCount > 0) { // there is at least one server is aborted
                    char arr[] = "ABORT";
                    for (auto& it : peersSocketDescriptorNOnlinestatus) {  // broadcast abort messages to all the peers
                        send(it.first, arr, sizeof(arr), 0);
                        if (D) {
                            std::string logMessage = "Commit phase: master server broadcast the ABORT message to all the peers slave server, since there was one server can not perform write or replace operation \n";
                            logger(&logMessage[0]);
                        }
                    }
                    return 0;
                }
                else {
                    if (successCommitedPeersServerCount == peersSocketDescriptorNOnlinestatus.size()) {
                        // receives all the messages from slaveServers and return.
                        return 1;
                    } else {
                        char arr[] = "ABORT";
                        for (auto& it : peersSocketDescriptorNOnlinestatus) {  // broadcast abort messages to all the peers
                            send(it.first, arr, sizeof(arr), 0);
                            if (D) {
                                std::string logMessage = "Commit phase: master server broadcast the ABORT message to all the peers slave server, since there was one server can not perform write or replace operation \n";
                                logger(&logMessage[0]);
                            }
                        }
                        return 0;
                    }
                }
            }
            
        }
        
    }
  
    return 0;
}
