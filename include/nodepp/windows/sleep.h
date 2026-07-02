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

namespace nodepp { namespace process { using NODE_INTERVAL = ULARGE_INTEGER; }}
namespace nodepp { namespace process {

    inline NODE_INTERVAL get_time_interval(){ 
    thread_local static NODE_INTERVAL borrow   = {0};
    /*---------------*/ NODE_INTERVAL interval = {0};
    /*---------------*/ NODE_INTERVAL out      = {0};

        FILETIME ft; GetSystemTimeAsFileTime(&ft);

        interval.LowPart  = ft.dwLowDateTime ;
        interval.HighPart = ft.dwHighDateTime;

        if( borrow.QuadPart == 0 ){ borrow = interval; }
        out.QuadPart = interval.QuadPart - borrow.QuadPart;
        
        return out;
    }

    inline ulong micros(){ 
        NODE_INTERVAL interval = get_time_interval(); 
        return interval.QuadPart / 10; 
    }

    inline ulong millis(){ 
        NODE_INTERVAL interval = get_time_interval(); 
        return interval.QuadPart / 10000; 
    }

    inline ulong seconds(){ 
        NODE_INTERVAL interval = get_time_interval(); 
        return interval.QuadPart / 10000000; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline void delay( ulong time ){ ::Sleep(time); }

    inline ulong now (){ return millis(); }

    inline void yield(){ delay(1); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/