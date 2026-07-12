/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_SLEEP
#define NODEPP_POSIX_SLEEP

/*────────────────────────────────────────────────────────────────────────────*/

#include <unistd.h>
#include <sys/time.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { using NODE_INTERVAL = struct timeval; } }
namespace nodepp { namespace process { 

    inline NODE_INTERVAL get_time_interval(){ 
    thread_local static NODE_INTERVAL borrow   = {0};
    /*---------------*/ NODE_INTERVAL interval = {0};
    /*---------------*/ NODE_INTERVAL out      = {0};

        gettimeofday(&interval, NULL);
        if( borrow.tv_usec==0 && borrow.tv_sec==0 ){ borrow = interval; }

        out.tv_sec  = interval.tv_sec  - borrow.tv_sec ;
        out.tv_usec = interval.tv_usec - borrow.tv_usec;

        if( out.tv_usec < 0 ){ out.tv_sec--; out.tv_usec += 1000000; }
        return out;
    }
    
    inline ulong micros(){ NODE_INTERVAL time = get_time_interval();
        return time.tv_sec * 1000000 + time.tv_usec; 
    }
    
    inline ulong seconds(){ NODE_INTERVAL time = get_time_interval();
        return time.tv_sec + time.tv_usec / 1000000; 
    }
    
    inline ulong millis(){ NODE_INTERVAL time = get_time_interval();
        return time.tv_sec * 1000 + time.tv_usec / 1000; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline void delay( ulong time ){ ::usleep(time*1000); }

    inline ulong now (){ return millis(); }

    inline void yield(){ delay(1); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/