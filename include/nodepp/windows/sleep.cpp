/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    struct node_time { FILETIME ft; ULARGE_INTEGER time; } _time_;

    ulong seconds(){ return _time_.time.QuadPart / 10000000; }

    ulong millis(){ return _time_.time.QuadPart / 10000; }

    ulong micros(){ return _time_.time.QuadPart / 10; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    ulong now(){ return millis(); }

    void delay( ulong time ){ 
        if( time == 0 ){ return; }
        ::Sleep( time ); 
    }

    void yield(){ GetSystemTimeAsFileTime(&_time_.ft);
        _time_.time.HighPart = _time_.ft.dwHighDateTime;
        _time_.time.LowPart  = _time_.ft.dwLowDateTime;
        delay( TIMEOUT ); 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/