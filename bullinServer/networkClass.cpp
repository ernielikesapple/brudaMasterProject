//
//  networkClass.cpp
//  bullinServer
//
//  Created by Ernie on 2020-06-06.
//  Copyright © 2020 Ernie. All rights reserved.
//

#include "networkClass.hpp"
#include <iostream>
#include <unistd.h>

using namespace std;

//https://www.youtube.com/watch?v=gyA7uDlazkc how to seperate code into .hpp and .cpp
networkClass::networkClass() {  // constructor

}

void networkClass:: sendRequest() {
    
    cout << "sendRequest from another class" << endl;
    
};


int networkClass:: run_it (char* command, char* argv [], char *envp[]) // a function which duplicates the current process,
{// 1st para char * command is a pointer to the char,
    int childp = fork(); // The fork system call creates a new process but that process contains, and is executing, exactly the same code that the parent process has.
    int status = 0;
    
    if (childp == 0) {  // the child process,
        cout << "111======" << childp << endl;
        execve(command, argv, envp);
        //exit(0); // always safer to follow a line of this, just in case the execve fails, the code in the else will execute again

        
    }
    else {

    waitpid(childp, &status, 0);  // para dans waitpid  https://support.sas.com/documentation/onlinedoc/sasc/doc/lr2/waitpid.htm
    cout << "222===" << childp  << "-----" << &status << endl;
    }
    return status;
}


char** networkClass:: pointersFunctions(int buffersize) // the int is the return type, it can be  void too
{
    char* buffer  = new char[buffersize]; // char is one byte, ask 8 bytes of memory, and returing a pointer to the beginning of that block of memory,
    memset(buffer,1, buffersize); // fills a block of memory with the data we specify , 1st parameter shows where, second para value, third, how many bytes should it filled
    
    char **pointer = &buffer;
    
    delete [] buffer;
    return pointer;
}





