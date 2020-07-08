//
//  TcpUtils.cpp
//  bullinServer
//
//  Created by Ernie on 2020-07-08.
//  Copyright © 2020 Ernie. All rights reserved.
//

#include "TcpUtils.hpp"
#include "fstream"
#include <iostream>


using namespace std;

#define NUll_POINTER 0

TcpUtils* TcpUtils::singleton = NUll_POINTER;


TcpUtils* TcpUtils::newAInstance() {
    if (singleton == NUll_POINTER) {
        singleton = new TcpUtils;
    }
    return singleton;
}



void TcpUtils::printhelpFunction(void)
{
    printf("\n Usage: [OPTIONS] [Argh]\n\n");
    printf("  Options:\n");
    printf("   -b arg  (string)       overrides (or sets) the file name bbfile according to its argument\n");
    printf("   -c arg  (string)       overrides (or sets) the file name bbserv.conf  according to its argument\n");
    printf("   -T arg  (int)          overrides Tmax according to its argument\n");
    printf("   -t arg  (int)          overrides Tmax according to its argument\n");
    printf("   -p arg  (int)          overrides the port number bp according to its argument \n");
    printf("   -s arg  (int)          overrides the port number sp according to its argument \n");
    printf("   -f                     (with no argument) forces d to false or 0 \n");
    printf("   -d                     (with no argument) forces D to true or 1 \n");
    printf("   host:port              Any non-switch argument is further interpreted as a peer specification (and so must have the form host:port as explained above) \n");
    printf("\n");
}
