#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <string>

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
    string input = "";

    // Retrieve the (non-option) argument:
    if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') ) {  // there is NO input...
        cerr << "No argument provided!" << endl;
        //return 1;
    }
    else {  // there is an input...
        input = argv[argc-1];
        // TODO: deal with  non-option arguments: https://stackoverflow.com/questions/18079340/using-getopt-in-c-with-non-option-arguments
    }

    cout << "input = " << input << endl;
    cout << "argv的原貌" << argv[argc-1]  << endl;

    // Shut GetOpt error messages down (return '?'):
    opterr = 0;

    // Retrieve the options:
    while ( (option = getopt(argc, argv, "b:c:T:t:p:s:fdh")) != -1 ) {  // for each option...
        switch ( option ) {
            case 'b':
                cout << "ToDo: change bbfile new name" << optarg << endl;
                break;
            case 'c':
                cout << "ToDo: change bbserv.conf  new name" << optarg << endl;
                break;
            case 'T':
                Tmax = stoi(optarg);
                cout << "do things with T para, new Tmax now is: " << Tmax << endl;
                break;
            case 't':
                Tmax = stoi(optarg);
                cout << "do things with T para, new Tmax now is: " << Tmax << endl;
                break;
            case 'p':
                bp = stoi(optarg);
                cout << "do things with bp para, new bp now is: " << bp << endl;
                break;
            case 's':
                sp = stoi(optarg);
                cout << "do things with sp para, new bp now is: " << sp << endl;
                break;
            case 'f':
                d = false;
                cout << "ToDo: change value of d in conf file" << endl;
                break;
            case 'd':
                D = true;
                cout << "ToDo: change value of D in conf file" << endl;
                break;
            case '?':  // unknown option...
                    cerr << "Unknown option: '" << char(optopt) << "'!" << endl;
                break;
            case 'h':
                printhelpFunction();
                return EXIT_SUCCESS;
            default:
                cout << "ToDo: handle non-switch argument " << endl;
                cout << "ToDo: handle error" << endl;
        }
    }
    
    
    
    return 0;
}
