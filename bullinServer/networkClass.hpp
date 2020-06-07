//
//  networkClass.hpp
//  bullinServer
//
//  Created by Ernie on 2020-06-06.
//  Copyright © 2020 Ernie. All rights reserved.
//

#ifndef networkClass_hpp
#define networkClass_hpp


#include <stdio.h>

class networkClass {
public:
    networkClass();
public:
    void sendRequest();
public:
    int mutiplierFunctions(int a, int b);
public:
    int run_it (char* command, char* argv [], char *envp[]);

};
#endif /* networkClass_hpp */
