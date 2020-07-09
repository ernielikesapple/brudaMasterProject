#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <algorithm>


#include "TcpUtils.hpp"
#include "ConfigFileHandler.hpp"

using namespace std;

static string configFileName = "bbserv.conf";
static int Tmax = 20;   // keyValue in the config file: THMAX   // use stoi(), change int to string
static int bp = 9000;   // keyValue in the config file: BBPORT , client server port number
static int sp = 10000;  // keyValue in the config file: SYNCPORT, server port number
static string bbfile = "bbfile";   // keyValue in the config file: BBFILE               // config fileName mandatory data if it's empty then refuse to start the server,
static vector<string> peers;    // keyValue in the config file: PEERS
bool d = true;  // keyValue in the config file: DAEMON
bool D = false; // keyValue in the config file: DEBUG


int main(int argc, char** argv) {
    
    // all the singleTon
    TcpUtils* tcpUtils = TcpUtils::newAInstance();
    ConfigFileHandler* configFileHandler = ConfigFileHandler::newAInstance();
    // TODO: Build a config file reader to read the file content , load the value into each variables
    
    // TODO: CHECK IF THE Config file exists, and if not exit the program and throw error
    
    
    
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
    while (optind < argc) {
        if ( (option = getopt(argc, argv, "b:c:T:t:p:s:fdh")) != -1 ) {  // for each option...
            switch ( option ) {
                case 'b':  // change bbfile name
                    configFileHandler -> configFileModifier(configFileName,"BBFILE",optarg);
                    // TODO: rename the bbfile fileName bbfile to sth else, search file renaming
                    //TODO: renaming.... note don't do this // bbfile = optarg; // now it will affect other command line
                    break;
                case 'c':  // change config file name
                    // TODO: rename config fileName bbfile to sth else, search file renaming
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
            optind++;
        }
    }
    
    // TODO: handle host:post  peers          and other start options
    
    
    // TODO: change the value in the config file if the value for d is true then we need to start the server
    
    // TODO: DAEMONZING
    
    int sd;
    sd = tcpUtils -> connectbyportint("www.google.com", 80);
    cout << "sock descriptor is" << sd << endl;
    return 0;
}
