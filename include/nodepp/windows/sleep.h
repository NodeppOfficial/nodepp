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

namespace nodepp { namespace process {

    inline uchar_64 start_sleep_machine(){ 
    uchar_64 out = 0; ULARGE_INTEGER* tmp = (ULARGE_INTEGER*) &out;

        FILETIME ft; GetSystemTimeAsFileTime(&ft);

        tmp->LowPart  = ft.dwLowDateTime ;
        tmp->HighPart = ft.dwHighDateTime;

    return out / 10; }

    inline uchar_64 get_time_interval(){ 
    thread_local static uchar_64 borrow   = start_sleep_machine();
    thread_local static uchar_64 stamp    = 0;
    /*---------------*/ uchar_64 interval = start_sleep_machine();

        if( borrow > interval ){

            stamp += interval + ( (uchar_64)-1 ) - borrow;
            borrow = interval ;

        } else {
            
            stamp += interval - borrow;
            borrow = interval ;

        }
        
    return stamp; }

    inline uchar_64  micros(){ return get_time_interval(); }

    inline uchar_64  millis(){ return get_time_interval() / 1000; }

    inline uchar_64 seconds(){ return get_time_interval() / 1000000; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    inline void  delay( ulong time ){ ::Sleep(time); }

    inline uchar_64 now (){ return millis(); }

    inline void  yield(){ delay(1); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/