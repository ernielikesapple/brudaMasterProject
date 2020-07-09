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

class ConfigFileHandler {
    
    private:
        static ConfigFileHandler* singleton;
    
    public:
        static ConfigFileHandler* newAInstance();
        
        void configFileModifier(std::string filename, std::string keyValueToBeSearched, std::string valueToBeFilled);
    
        void configFileReader(std::string filename);
        
};



#endif /* ConfigFileHandler_hpp */
