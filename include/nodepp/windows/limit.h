/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_LIMIT
#define NODEPP_WINDOWS_LIMIT

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>
#include "os.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {
  
    inline uint get_max_cpus_threads(){ 
        return os::cpus(); 
    }

    inline int set_max_cpus_threads( int size ){
        return size<=os::cpus() ? 1 : -1;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {
    
    inline int set_hard_fileno( uint value ) { return _setmaxstdio( value ); }

    inline int set_soft_fileno( uint value ) { return _setmaxstdio( value ); }
    
    /*─······································································─*/

    inline uint get_hard_fileno() { return _getmaxstdio(); }

    inline uint get_soft_fileno() { return _getmaxstdio(); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {

    inline uint get_hard_thread_pool() {
        MEMORYSTATUSEX status; status.dwLength = sizeof(status);
        if( GlobalMemoryStatusEx(&status) ) 
          { return (uint)((status.ullTotalVirtual/(1024*1024))*0.9); }
    return 1024; }

    inline uint get_soft_thread_pool() {
        SYSTEM_INFO sysinfo; GetNativeSystemInfo( &sysinfo );
        return (uint)sysinfo.dwNumberOfProcessors;
    }

    inline int set_hard_thread_pool( uint value ) {
        return ( value <= get_hard_thread_pool() ) ? 0 : -1;
    }

    inline int set_soft_thread_pool( uint value ) {
        return ( value <= get_hard_thread_pool() ) ? 0 : -1;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {
    
    enum PRIORITY {
         IDLE_PRIORITY    , LOW_PRIORITY     ,
         NORMAL_PRIORITY  , HIGH_PRIORITY    ,
         HIGHEST_PRIORITY , REALTIME_PRIORITY
    };

    inline int set_process_priority( int priority ){ DWORD n; switch( priority ) {
             case IDLE_PRIORITY:     n=IDLE_PRIORITY_CLASS;         break;
             case LOW_PRIORITY:      n=BELOW_NORMAL_PRIORITY_CLASS; break;
             case NORMAL_PRIORITY:   n=NORMAL_PRIORITY_CLASS;       break;
             case HIGH_PRIORITY:     n=ABOVE_NORMAL_PRIORITY_CLASS; break;
             case HIGHEST_PRIORITY:  n=HIGH_PRIORITY_CLASS;         break;
             case REALTIME_PRIORITY: n=REALTIME_PRIORITY_CLASS;     break; default: return -1;
        }    return SetPriorityClass( GetCurrentProcess(), n ) ? priority : -1;
    }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/
