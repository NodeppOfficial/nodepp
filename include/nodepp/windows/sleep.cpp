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
#include <windows.h>

struct NODE_INTERVAL { FILETIME ft; ULARGE_INTEGER time; };

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    NODE_INTERVAL get_new_interval(){ 
        NODE_INTERVAL interval; GetSystemTimeAsFileTime( &interval.ft );
        interval.time.HighPart = interval.ft.dwHighDateTime;
        interval.time.LowPart  = interval.ft.dwLowDateTime;
        return interval;
    }

    ulong micros(){ 
        NODE_INTERVAL interval = get_new_interval(); 
        return interval.time.QuadPart / 10; 
    }

    ulong millis(){ 
        NODE_INTERVAL interval = get_new_interval(); 
        return interval.time.QuadPart / 10000; 
    }

    ulong seconds(){ 
        NODE_INTERVAL interval = get_new_interval(); 
        return interval.time.QuadPart / 10000000; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    void delay( ulong time ){ ::Sleep( time ); }

    void yield(){ delay( TIMEOUT ); }

    ulong now(){ return millis(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/