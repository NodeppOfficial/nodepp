/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_SLEEP
#define NODEPP_WINDOWS_SLEEP

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { struct NODE_INTERVAL { FILETIME ft; ULARGE_INTEGER time; }; } }
namespace nodepp { namespace process {

    inline NODE_INTERVAL& get_new_interval(){ 
        thread_local static NODE_INTERVAL interval; GetSystemTimeAsFileTime( &interval.ft );
        interval.time.HighPart = interval.ft.dwHighDateTime;
        interval.time.LowPart  = interval.ft.dwLowDateTime;
        return interval;
    }

    inline ulong micros(){ 
        NODE_INTERVAL interval = get_new_interval(); 
        return interval.time.QuadPart / 10; 
    }

    inline ulong millis(){ 
        NODE_INTERVAL interval = get_new_interval(); 
        return interval.time.QuadPart / 10000; 
    }

    inline ulong seconds(){ 
        NODE_INTERVAL interval = get_new_interval(); 
        return interval.time.QuadPart / 10000000; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline ulong set_timeout( ulong time=0 ) {
        if( time == -1 ) /*------------*/ { return 0; }
    thread_local static ulong stamp; ulong out=stamp;
        if( stamp > time || stamp == 0 ){ stamp=time; }
    return out==0 ? 1 : out; }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline void delay( ulong time ){ ::Sleep(time); }

    inline void yield(){ delay(TIMEOUT); }

    inline ulong now(){ return millis(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/