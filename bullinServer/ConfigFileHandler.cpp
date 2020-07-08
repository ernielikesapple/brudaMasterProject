//
//  ConfigFileHandler.cpp
//  c++PlayGround
//
//  Created by Ernie on 2020-07-07.
//  Copyright © 2020 Ernie. All rights reserved.
//

#include "ConfigFileHandler.hpp"
#include "fstream"
#include <iostream>


using namespace std;

#define NUll_POINTER 0

ConfigFileHandler* ConfigFileHandler::singleton = NUll_POINTER;


ConfigFileHandler* ConfigFileHandler::newAInstance() {
    if (singleton == NUll_POINTER) {
        singleton = new ConfigFileHandler;
    }
    return singleton;
}

void ConfigFileHandler::configFileOpener(string filename) {
    
    // append data to a file
    ofstream fileInstance; // notice we can write ofstream or fstream, ofstream for explicitly say we are output info into file, and notice
    fileInstance.open("bbserv.conf",std::ios_base::app); // notice we need to add the postfix at the end of the bbserv file not create it directly in IDE, and std::ios_base::app, for append the text in the end
    fileInstance << "as1dasd";
    fileInstance.close();
    
    
    // read data into a file
    string line;
    ifstream myfile ("bbserv.conf");
    if (myfile.is_open()) {
      cout << "open file success" << '\n';
      while ( getline (myfile,line) )
      {
        cout << line << '\n';
      }
      myfile.close();
    } else {
      cout << "open file fail" << '\n';
    }
}
