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

using TIMEVAL = struct timeval;

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { TIMEVAL timeval;
    
    ulong  millis(){ return timeval.tv_sec * 1000 + timeval.tv_usec / 1000; }
    ulong seconds(){ return timeval.tv_sec + timeval.tv_usec / 1000000; }
    ulong  micros(){ return timeval.tv_sec * 1000000 + timeval.tv_usec; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    void delay( ulong time ){ if( time==0 ){ return; } ::usleep( time*1000 ); }

    void yield(){ delay( TIMEOUT ); gettimeofday( &timeval, NULL ); }

    ulong now(){ return millis(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/