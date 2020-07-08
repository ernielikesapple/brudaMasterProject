//
//  TcpUtils.hpp
//  bullinServer
//
//  Created by Ernie on 2020-07-08.
//  Copyright © 2020 Ernie. All rights reserved.
//

#ifndef TcpUtils_hpp
#define TcpUtils_hpp

#include <stdio.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*** Error codes: ***/

/* See below for what they mean. */
const int err_host    = -1;
const int err_sock    = -2;
const int err_connect = -3;
const int err_proto   = -4;
const int err_bind    = -5;
const int err_listen  = -6;


class TcpUtils {
    
    private:
        static TcpUtils* singleton;
    
    public:
        static TcpUtils* newAInstance();
        
        int connectbyportint(const char* host, const unsigned short port);
        
        void printhelpFunction(void);
};


#endif /* TcpUtils_hpp */
