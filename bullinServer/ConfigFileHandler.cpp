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
#include <algorithm>

using namespace std;

#define NUll_POINTER 0

ConfigFileHandler* ConfigFileHandler::singleton = NUll_POINTER;


ConfigFileHandler* ConfigFileHandler::newAInstance() {
    if (singleton == NUll_POINTER) {
        singleton = new ConfigFileHandler;
    }
    return singleton;
}

void ConfigFileHandler::configFileModifier(string filename, string keyToBeSearched, string valueToBeFilled) {
    fstream configFile;
    configFile.open(filename.c_str(),ios::in);
    if (configFile.fail()) {
        cout << "Unable to find defaultConfig file" << endl;
        exit(0);
    }
    string configFileContainer = "";
    string delimeter = "=";
    
    string line;
    while ( getline (configFile,line) )
    {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace),line.end()); // remove the white space
        if(line[0] == '#' || line.empty()) continue; // skip comment
        size_t foundComments = line.find_first_of('#');  // if not find the # it will return a very large number so that the next line can the whole text,
        string configStringInEachLine = line.substr(0, foundComments);
        
        // TODO: notice to have, add trim and reduce
        string key, value;
        unsigned long foundDelimeter = configStringInEachLine.find(delimeter);
        if (foundDelimeter != string::npos) {
            key  = configStringInEachLine.substr(0, foundDelimeter);
            value = configStringInEachLine.substr(foundDelimeter+1);
            
            if (key == keyToBeSearched) {
                if (value.length() > valueToBeFilled.length()) {
                    line.replace(foundDelimeter+1, value.length(), valueToBeFilled);
                } else {
                    line.replace(foundDelimeter+1, valueToBeFilled.length(), valueToBeFilled);
                }
            }
            configFileContainer = configFileContainer + line + "\n";
        }
    }
    configFile.close();
    
    configFile.open(filename.c_str(),ios::out);  // replace all the text in the text file again
    if (configFile.fail()) {
        cout << "Unable to find defaultConfig file" << endl;
        exit(0);
    }
    
    configFile << configFileContainer;
    configFile.close();
    
}

void ConfigFileHandler::configFileReader(std::string filename) { // TODO: Refactoring the code,
    fstream configFile;
    configFile.open(filename.c_str(),ios::in);
    if (configFile.fail()) {
        cout << "Unable to find defaultConfig file" << endl;
        exit(0);
    }
    string configFileContainer = ""; // TODO: Refactoring the code, change it to a map to store value
    string delimeter = "=";
    
    string line;
    while ( getline (configFile,line) )
    {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace),line.end()); // remove the white space
        if(line[0] == '#' || line.empty()) continue; // skip comment
        
        size_t foundComments = line.find_first_of('#');  // if not find the # it will return a very large number so that the next line can the whole text,
        string configStringInEachLine = line.substr(0, foundComments);
        
        // TODO: notice to have, add trim and reduce
        
        string key, value;
        unsigned long foundDelimeter = configStringInEachLine.find(delimeter);
        if (foundDelimeter != string::npos) {
            key  = configStringInEachLine.substr(0, foundDelimeter);
            value = configStringInEachLine.substr(foundDelimeter+1);
            // TODO: store key value pair into map
        }
    }
    configFile.close();
    
    // TODO: Notice when handling the boolean the value can be 0 / 1 / true / false, read the pdf again
        
}
