#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <algorithm>


#include "tcp-utils.h"

using namespace std;

static int Tmax = 20;
static int bp = 9000;
static int sp = 10000;
static FILE *bbfile;                // mandatory data if it's empty then refuse to start the server
static vector<string> peers;
bool d = true;
bool D = false;

int main(int argc, char** argv) {
    
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

    cout << "lastInput = " << lastInput << endl;

    // Shut GetOpt error messages down (return '?'):
    opterr = 0;

    // Retrieve the options:
    while (optind < argc) {
        if ( (option = getopt(argc, argv, "b:c:T:t:p:s:fdh")) != -1 ) {  // for each option...
            switch ( option ) {
                case 'b':  // change bbfile name
                    cout << "ToDo: change bbfile new name" << optarg << endl;
                    break;
                case 'c':  // change config file name
                    cout << "ToDo: change bbserv.conf  new name" << optarg << endl;
                    break;
                case 'T':  // overide Tmax number, preallocated threads
                    Tmax = stoi(optarg);
                    cout << "do things with T para, new Tmax now is: " << Tmax << endl;
                    break;
                case 't':  // overide Tmax number, preallocated threads
                    Tmax = stoi(optarg);
                    cout << "do things with T para, new Tmax now is: " << Tmax << endl;
                    break;
                case 'p':  // client server port number
                    bp = stoi(optarg);
                    cout << "do things with bp para, new bp now is: " << bp << endl;
                    break;
                case 's': // server port number
                    sp = stoi(optarg);
                    cout << "do things with sp para, new bp now is: " << sp << endl;
                    break;
                case 'f':   // start daemon...
                    d = false;
                    cout << "ToDo: change value of d in conf file" << endl;
                    break;
                case 'd':   // debug mode...
                    D = true;
                    cout << "ToDo: debug mode" << endl;
                    break;
                case '?':  // unknown option...
                        cerr << "Unknown option: '" << char(optopt) << "'!" << endl;
                    break;
                case 'h':  // help menu
                    printhelpFunction();
                    return EXIT_SUCCESS;
                default:
                    cout << "ToDo: handle non-switch argument " << endl;
                    cout << "ToDo: handle error" << endl;
                    break;
            }
        } else { // handle host:post  peers          and other start options
            cout << "handle non-switch argument are: " << argv[optind] << endl;
            optind++;
        }
    }
    
    
    // change the value in the config file if the value for d is true then we need to start the server
    
    
    
    return 0;
}
