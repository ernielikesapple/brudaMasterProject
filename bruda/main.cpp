#include <iostream>
#include "tcp-utils.h"

using namespace std;

int main()
{

    int u;
    u = connectbyportint("www.googlrrre.com", 80);
    cout << "Hello worldasd!" << u << endl;
    return 0;
}
