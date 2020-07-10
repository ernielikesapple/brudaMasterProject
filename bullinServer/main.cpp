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

#include "TcpUtils.hpp"
#include "ConfigFileHandler.hpp"

using namespace std;

string configFileName = "bbserv.conf";
string Tmax = "20";   // keyValue in the config file: THMAX   // use stoi(), change int to string
string bp = "9000";   // keyValue in the config file: BBPORT , client server port number
string sp = "10000";  // keyValue in the config file: SYNCPORT, server port number
string bbfile = "bbfile";   // keyValue in the config file: BBFILE               // config fileName mandatory data if it's empty then refuse to start the server,
string peers = "";    // keyValue in the config file: PEERS // TODO: to verify the value inside the server function make sure it has value
bool daemonMode = true;   // keyValue in the config file: DAEMON
bool D = false; // keyValue in the config file: DEBUG

/*
DOUBLE CHECK ME!!!!
This is preliminary documentation of what I am thinking for network protocol and what to save in the 'database'
*/
//class message
//private
    //int lockedByThreadID;

//public
    //Master transmit
    //uint32 messageNumber; //DB row 0; Can't expect our servers are using the exact current time for a message index so this
    //time_t timestamp;     //When masterPeer received using POSIX time used for timeout start of 10 seconds
    //peer masterPeer;      //This will simplify design because messages tell us how and who to talk to
    //uint8_t status (enumerated int value)
    //string message;       //DB row 1;
    //uint16_t msgLen

    //Slave Responce
    //uint32 messageNumber
    //uint8_t status (enumerated int value)
    //uint16_t msgSum; //To check our data, add the int value of each char together for a total

    /*
    Abort message
    uint8_t type
        TIMEOUT,
        TAKEN_NUMBER,
        BAD_DATA
    char[]
        ip/port,
        number taken,
        data
    uint16_t 
        length
    */

    /*
    PRO TIP
    it's going to take some extra work to match up our peer using ip/port instead of linking with a pointer or 
    using the same order with have the peers list in. That is a bug waiting to happen. Someone is going to sort the
    list or insert a peer in the middle etc not understanding order matters. Worse, pointers can be exploited to crash
    the server or worse. In network code I almost never use pointers. To that end even though we store lenght for things
    I usually count again and compare to our stored value. If there is a missmatch I drop the packet or at least log it
    because something fishy is going on.
    */
    //peer list status
    //array [ip, port, TX/RX, status]
        //CREATE
        //COMMIT
        //COMPLETE
        //ABORT

    /*
    Master and slave each have 3 steps
    1
    Master:
        Send all info about the message
    Slave:
        Allocate a message and check it against current messages to make sure there isn't already 
        a message #78 for example in the process of being created.
    2
    Master:
        Broadcast commit
    Slave:
        Write to database and send conformation of completion
    3
    Master:
        If all slaves completed, broadcast complete
    Slave:
        If final complete isn't recieved, remove database entry
    */



// all the singleTon
TcpUtils* tcpUtils = TcpUtils::newAInstance();
ConfigFileHandler* configFileHandler = ConfigFileHandler::newAInstance();

void loadConfigFile() {
    if (std::ifstream(configFileName)) { // config file exists
        configFileHandler ->configFileReader(configFileName); // read all the value from config file and load them in a map inside configFileHandler
        
        configFileHandler ->configFileValueGetter("THMAX", Tmax);  // get all the value from the map into all the variabbles on this page
        configFileHandler ->configFileValueGetter("BBPORT", bp);
        configFileHandler ->configFileValueGetter("SYNCPORT", sp);
        configFileHandler ->configFileValueGetter("BBFILE", bbfile);
        if (!std::ifstream(bbfile)) { // make sure the bbfile exist, so the server can start up normally
            cerr << "bbfile Not provided! Notice you must have a bbfile under current directory and Notice you should name the bbfile file like 'bbfile' " << endl;
            exit(0);
        }
        configFileHandler ->configFileValueGetter("PEERS", peers); // TODO: to verify the value inside the server function make sure it has value
        string tempStringBoolean_d = "";
        configFileHandler ->configFileValueGetter("DAEMON", tempStringBoolean_d);
        if (tempStringBoolean_d == "true" || tempStringBoolean_d == "1") {
            d = true;
        } else {
            d = false;
        }
        string tempStringBoolean_D = "";
        configFileHandler ->configFileValueGetter("DEBUG", tempStringBoolean_D);
        if (tempStringBoolean_D == "true" || tempStringBoolean_D == "1") {
            d = true;
        } else {
            d = false;
        }
    } else {
        cerr << "config file Not provided! Notice you should name the config file like 'bbserv.conf' " << endl;
        exit(0);
    }
}


int main(int argc, char** argv) {
    
    loadConfigFile();
    
    // handle command line arguments:
    int option;
    string lastInput = "";

    // Retrieve the (non-option) argument:
    if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') ) {  // there is NO input...
        cerr << "No argument provided!" << endl;
    }
    else {  // there is an input...
        lastInput = argv[argc-1];
    }
    
    // Retrieve the options:
    string peersString = "";
    while (optind < argc) {
        if ( (option = getopt(argc, argv, "b:c:T:t:p:s:fdh")) != -1 ) {  // for each option...
            switch ( option ) {
                case 'b':  // change bbfile name
                    cout << "bbfile value is----" << bbfile << "bbfile cstring is : " << bbfile.c_str() <<  "options is : " << optarg << endl;
                    std::rename(bbfile.c_str(), optarg);
                    configFileHandler -> configFileModifier(configFileName,"BBFILE",optarg); // insert the new name inside the config file
                    break;
                case 'c':  // change config file name
                    std::rename(configFileName.c_str(), optarg);
                    configFileName = optarg; // TODO: VERIFY THIS with the signalhug part again insert the new value into this configFile variable so that when reload it will use the new name
                    break;
                case 'T':  // overide Tmax number, preallocated threads
                    configFileHandler -> configFileModifier(configFileName,"THMAX",optarg);
                    break;
                case 't':  // overide Tmax number, preallocated threads
                    configFileHandler -> configFileModifier(configFileName,"THMAX",optarg);
                    break;
                case 'p':  // client server port number
                    configFileHandler -> configFileModifier(configFileName,"BBPORT",optarg);
                    break;
                case 's': // server port number
                    configFileHandler -> configFileModifier(configFileName,"SYNCPORT",optarg);
                    break;
                case 'f':   // start daemon...
                    configFileHandler -> configFileModifier(configFileName,"DAEMON","false");
                    break;
                case 'd':   // debug mode...
                    configFileHandler -> configFileModifier(configFileName,"DEBUG","true");
                    break;
                case '?':  // unknown option...
                        cerr << "Unknown option: '" << char(optopt) << "'!" << endl;
                    break;
                case 'h':  // help menu
                    tcpUtils -> printhelpFunction();
                default:
                    break;
            }
        } else {
            // TODO: use a string to append/take all the values from argv[optind] , and then after the while loop take everything into the config file
            cout << "handle non-switch argument are: " << argv[optind] << endl;
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
    
    // TODO: change the value in the config file if the value for d is true then we need to start the server
    
        if(daemonMode) {
        while(exitRequested == 0 && packetQueCount < 1) {
            //setup sockets, allocate threads
            //if(serverIsInitialized == false)
                //initializeServer();
            
            //get packets out of the system's que and into our packetQue
            //updatePacketQue();

            /*
            THREADING PLAN
            As far as I can tell from the documentation there is no lower limit on the number of threads we can have.
            That puts a lot of limits on the robustness of our multicore effiecency. That's not a bad thing because
            threading is hard to design, write and debug. We have an excuse to keep it simple.     
            
            I am thinking 10 times a second we look for new packets. Each new packet gets sent to a free thread to work on a
            message. While a thread is working on a message it is off limits to other threads. If this was really a high 
            performance server we would poll more often but it's not. If we have no threads we will just process each packet
            in a loop in the traditional manor but still wait so we arn't just executing our packet checking code millions of
            times a second.
            */
        }
    }
    return 0;
}
