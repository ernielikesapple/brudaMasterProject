//
//  ConfigFileHandler.hpp
//  c++PlayGround
//
//  Created by Ernie on 2020-07-07.
//  Copyright © 2020 Ernie. All rights reserved.
//

#ifndef ConfigFileHandler_hpp
#define ConfigFileHandler_hpp

#include <stdio.h>
#include <iostream>
#include <map>

class ConfigFileHandler {
    
    private:
        static ConfigFileHandler* singleton;
        std::map<std::string, std::string> configDataMap;
    public:
        static ConfigFileHandler* newAInstance();
        
//        void configFileModifier(std::string filename, std::string keyValueToBeSearched, std::string valueToBeFilled);
    
        void configFileReader(std::string filename);
        void configFileValueGetter(std::string key, std::string& value);  // passing a reference so that value will be changed directly
        
};



#endif /* ConfigFileHandler_hpp */
