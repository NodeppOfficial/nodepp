/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once

#include <unistd.h>
#include <sys/time.h>

using NODE_INTERVAL = struct timeval;

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { 

    NODE_INTERVAL get_new_interval(){ NODE_INTERVAL interval;
        gettimeofday( &interval, NULL );
        return interval;
    }
    
    ulong micros(){ NODE_INTERVAL time = get_new_interval();
        return time.tv_sec * 1000000 + time.tv_usec; 
    }
    
    ulong seconds(){ NODE_INTERVAL time = get_new_interval();
        return time.tv_sec + time.tv_usec / 1000000; 
    }
    
    ulong millis(){ NODE_INTERVAL time = get_new_interval();
        return time.tv_sec * 1000 + time.tv_usec / 1000; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    void delay( ulong time ){ ::usleep( time * 1000 ); }

    void yield(){ delay( TIMEOUT ); }

    ulong now(){ return millis(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/