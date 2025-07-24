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

struct TIMEVAL { FILETIME ft; ULARGE_INTEGER time; };

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    TIMEVAL get_new_timeval(){ 
        TIMEVAL time; GetSystemTimeAsFileTime( &timeval.ft );
        timeval.time.HighPart = timeval.ft.dwHighDateTime;
        timeval.time.LowPart  = timeval.ft.dwLowDateTime;
        return timeval;
    }

    ulong  micros(){ get_new_timeval(); return timeval.time.QuadPart / 10; }

    ulong  millis(){ get_new_timeval(); return timeval.time.QuadPart / 10000; }

    ulong seconds(){ get_new_timeval(); return timeval.time.QuadPart / 10000000; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    void delay( ulong time ){ ::Sleep( time ); }

    void yield(){ delay( TIMEOUT ); }

    ulong now(){ return millis(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/