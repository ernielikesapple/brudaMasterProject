
#include <iostream>
#include <sys/types.h>

#include <sys/wait.h>

#include "networkClass.hpp"

using namespace std;

class test {
public:
    float a;
    void tesf() {
        cout << a << endl;
    }
};

int main(int argc, char** argv)
{
    test t;
    t.tesf();
    cout << t.a << endl;
    
    
    // example of calling a function in the same class
    
    networkClass nc; // declare an class
    nc.sendRequest();
    
    int result = nc.mutiplierFunctions(3, 3);
    
    char *command = "/bin/ls";
    char *args[] = {"ls", "-aF", "/", 0};    /* each element represents a command line argument */
    char *env[] = { 0 };    /* leave the environment list null */

    nc.run_it(command, args, env);
    cout << "==the result of mutiplierFunctions===" << result << endl;
    
   // nc.sendRequest();

    
}


