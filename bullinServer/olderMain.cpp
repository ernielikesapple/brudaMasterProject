//
//  olderMain.cpp
//  bullinServer
//
//  Created by Ernie on 2020-07-03.
//  Copyright © 2020 Ernie. All rights reserved.
//


#include "olderMain.hpp"

#include <iostream>
#include <sys/types.h>

#include <sys/wait.h>
#include "howStringWorks.c"
#include "networkClass.hpp"

#include <string>

#include <fstream>
#include <algorithm>

using namespace std;

class test {
public:
    float a;
    void tesf() {
        cout << a << endl;
    }
};

//void Increment(int value)  // 🌟very important, the difference between "int value" and "int* value" is , when you declare like  "int value",  when calling this function and pass the value into the parameter of the function, inside the function, the value is copy of the parameter, which is stored at a different memory address
// https://www.youtube.com/watch?v=IzoFn3dfsPA
void Increment(int* value) // example to show what is a references
{
    //value++; // if pass  *value into this function and without dereference
    /*
    cout << "=="<< *value << "==" << value <<"==" << &value << endl; // the *value is dereference the pointer, and get the value,,,,, the pointer which is an address number,   "&value is address of the integer pointer "int* value"
    *value++;
    cout << "=="<< *value << "==" << *value-- <<"== " << value <<"==" << &value << endl;
    */
    (*value)++; // dereference this integer pointer first and then make value add by one
}

void Increment1(int& value) // pass reference directly
{
    value++;
}

void charFunctions(char* thisIsAcharPointer)
{
    cout << thisIsAcharPointer << endl;
}

void PassStringAroundFunction (const string& thisIsAcharPointer)       // 🌟const string&, pass the para as an const ref, so that this funciton is read only, and
{
    cout << thisIsAcharPointer << endl;
}

void main1(int argc, char** argv)
{
    charFunctions("aa");
    howStringWorks();
    
    string thisIsAstring = "thisIsAstring";
    char* thisIsAstringMadeByCharArray = "thisIsAstring";
    
    PassStringAroundFunction(thisIsAstringMadeByCharArray);
    
    cout << thisIsAstring <<"==" << thisIsAstringMadeByCharArray  << endl;
    
    
    
    test t;
    t.tesf();
    cout << t.a << endl;
    
    int a = 5;
    cout << a <<"==" << &a << endl;
    Increment(&a); // &a is actually a reference to the above line of variable, it will pass this address to this (int * value), which takes a pointer which points to a integer value,
    cout << a <<"==" << &a << endl;
    
    Increment1(a); // the parameter a here is just a reference of the orignal variable, so it will be the modified the value from the same address
    cout << a <<"==" << &a << endl;
    
    int aa = 5;
    int bb = 8;
    int& ref = aa; // and each time when we create a reference, we have to initiate is at the same time, can do sth like  "int& ref;"
    ref = bb; // here we can not change the refence which it referred to, here it will just give the ref a new value 8
    
    
    // example of calling a function in the same class
    
    networkClass nc; // declare an class
    nc.sendRequest();
    
    char** result = nc.pointersFunctions(8);
    void* pointer = &result;
    cout << "==the result of pointersFunctions===|" << &result << "|==end==" << endl;
    
    
    char *command = "/bin/ls";
    char *args[] = {"ls", "-aF", "/", 0};    /* each element represents a command line argument */
    char *env[] = { 0 };    /* leave the environment list null */

    nc.run_it(command, args, env);
    cout << "==the result of mutiplierFunctions===" << result << endl;
    
   // nc.sendRequest();

    
}



