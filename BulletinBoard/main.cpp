#include <iostream>
#include <sys/types.h>
#include <unistd.h>


using namespace std;

int main(char* command, char* argv[], char* envp[])
{

    int child = fork();
    if (child == 0) {
        execve(command, argv, envp);
        cout << "1111" << endl;
    }
    else {
    cout << "222" << endl;
    }
    return 0;
}
