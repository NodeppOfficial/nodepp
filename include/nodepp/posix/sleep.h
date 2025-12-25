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

    inline NODE_INTERVAL& get_new_interval(){ 
        thread_local static NODE_INTERVAL interval;
        gettimeofday( &interval,NULL );
        return interval;
    }
    
    inline ulong micros(){ NODE_INTERVAL time = get_new_interval();
        return time.tv_sec * 1000000 + time.tv_usec; 
    }
    
    inline ulong seconds(){ NODE_INTERVAL time = get_new_interval();
        return time.tv_sec + time.tv_usec / 1000000; 
    }
    
    inline ulong millis(){ NODE_INTERVAL time = get_new_interval();
        return time.tv_sec * 1000 + time.tv_usec / 1000; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline ulong set_timeout( int time=0 ) { 
        if( time == -1 ) /*------------*/ { return 0; }
    thread_local static ulong stamp; ulong out=stamp;
        if( stamp > time || stamp == 0 ){ stamp=time; }
    return out==0 ? 1 : out; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline void delay( ulong time ){ ::usleep(time*1000); }

    inline void yield(){ delay(TIMEOUT); }

    inline ulong now(){ return millis(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/