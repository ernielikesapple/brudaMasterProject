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


class TcpUtils {
    
    private:
        static TcpUtils* singleton;
    
    public:
        static TcpUtils* newAInstance();
        
        
        void printhelpFunction(void);
};


#endif /* TcpUtils_hpp */
