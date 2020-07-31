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
        cout << "Unable to find defaultConfig file 1" << endl;
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
        
        // TODO: notic to have, add trim and reduce
        string key, value;
        unsigned long foundDelimeter = configStringInEachLine.find(delimeter);
        if (foundDelimeter != string::npos) {
            key  = configStringInEachLine.substr(0, foundDelimeter);
            value = configStringInEachLine.substr(foundDelimeter+1);
            
            if (key == keyToBeSearched) {
                // updat the value in the map!
                map<string, string>::iterator it;
                it = configDataMap.find(key);
                if (it != configDataMap.end()) {
                    it -> second = valueToBeFilled;
                } else {
                    cout << "Unable to find the key in the map varible, when updating the value" << endl;
                }
                    
                // update the value in the file 1
                if (value.length() > valueToBeFilled.length()) {
                    line.replace(foundDelimeter+1, value.length(), valueToBeFilled);
                } else {
                    line.replace(foundDelimeter+1, valueToBeFilled.length(), valueToBeFilled);
                }
            }
            // update the value in the file 2
            configFileContainer = configFileContainer + line + "\n";
        }
    }
    configFile.close();
    
    configFile.open(filename.c_str(),ios::out);  // replace all the text in the text file again
    if (configFile.fail()) {
        cout << "Unable to find defaultConfig file 2" << endl;
        exit(0);
    }
    
    configFile << configFileContainer;
    configFile.close();
    
}

void ConfigFileHandler::configFileReader(std::string filename) {
    fstream configFile;
    configFile.open(filename.c_str(),ios::in);
    if (configFile.fail()) {
        cout << "Unable to find defaultConfig file 3" << endl;
        exit(0);
    }
    string delimeter = "=";
    
    string line;
    while ( getline (configFile,line) )
    {
        if(line[0] == '#' || line.empty()) continue; // skip comment
        
        size_t foundComments = line.find_first_of('#');  // if not find the # it will return a very large number so that the next line can the whole text,
        string configStringInEachLine = line.substr(0, foundComments);
        
        // TODO: notice to have, add trim and reduce
        string key, value;
        unsigned long foundDelimeter = configStringInEachLine.find(delimeter);
        if (foundDelimeter != string::npos) {
            key  = configStringInEachLine.substr(0, foundDelimeter);
            value = configStringInEachLine.substr(foundDelimeter+1);
            
            // update value in the value in the map or create a new value for and insert into the map
            map<string, string>::iterator it;
            it = configDataMap.find(key);
            if (it != configDataMap.end()) {
                it -> second = value;
            } else {
                configDataMap.insert(std::pair<string, string>(key, value));
            }
            
        }
    }
    configFile.close();
}

void ConfigFileHandler::configFileValueGetter(std::string key, std::string& value) {
    map<string, string>::iterator it;
    it = configDataMap.find(key);
    if (it != configDataMap.end()) {
        value = (it -> second).c_str();
    } else {
        cout << "Unable to find the key in the Config file" << endl;
    }
}
