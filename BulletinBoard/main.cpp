#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


using namespace std;

int main(char* command, char* argv[], char* envp[])
{

    int childp = fork();
    int status;

    if (childp == 0) {
        execve(command, argv, envp);
        //exit(0); // always safer to follow a line of this, just in case the execve fails, the code in the else will execute again

        cout << "111======" << childp << endl;
    }
    else {

    waitpid(childp, &status, 0);
    cout << "222===" << childp  << "-----" << &status << endl;
    }
    return status;
}
