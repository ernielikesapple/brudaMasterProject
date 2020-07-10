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
bool d = true;  // keyValue in the config file: DAEMON
bool D = false; // keyValue in the config file: DEBUG

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
    
    // TODO: DAEMONZING
    
    int sd;
    sd = tcpUtils -> connectbyportint("www.google.com", 80);
    cout << "sock descriptor is" << sd << endl;
    return 0;
}
