/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_OS
#define NODEPP_WINDOWS_OS

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    inline string_t hostname(){
        char buffer[NODEPP_UNBFF_SIZE]; DWORD bufferSize = NODEPP_UNBFF_SIZE;
        GetComputerNameA(buffer,&bufferSize); return string_t( buffer, bufferSize );
    }
    
    /*─······································································─*/

    inline string_t user(){ 
        char buffer[NODEPP_UNBFF_SIZE]; DWORD bufferSize = NODEPP_UNBFF_SIZE;
        GetUserNameA(buffer, &bufferSize); return string_t( buffer, bufferSize );
    }
    
    /*─······································································─*/

    inline string_t cwd(){ char buffer /**/ [NODEPP_UNBFF_SIZE];
        DWORD length = GetCurrentDirectoryA( NODEPP_UNBFF_SIZE, buffer );
        return string_t( buffer, length );
    }
    
    /*─······································································─*/

    inline uint cpus(){ 
        SYSTEM_INFO sysInfo; GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
    }
    
    /*─······································································─*/

    inline string_t tmp(){ string_t tmp (MAX_PATH);
        GetTempPathA( MAX_PATH, tmp.data() );
        return tmp;
    }
    
    /*─······································································─*/

    inline int exec( string_t cmd ){ return ::system( cmd.get() ); }

    inline int call( string_t cmd ){ return ::system( cmd.get() ); }

    /*─······································································─*/

    inline uint pid(){ return GetCurrentProcessId(); }
    
    /*─······································································─*/

    inline DWORD error(){ return GetLastError(); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
  
    inline uint get_max_cpus_threads(){ 
        return os::cpus(); 
    }

    inline int set_max_cpus_threads( uint size ){
        return size<=os::cpus() ? 1 : -1;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    inline int set_hard_fileno( uint value ) { return _setmaxstdio( value ); }

    inline int set_soft_fileno( uint value ) { return _setmaxstdio( value ); }
    
    /*─······································································─*/

    inline uint get_hard_fileno() { return _getmaxstdio(); }

    inline uint get_soft_fileno() { return _getmaxstdio(); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {

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

namespace nodepp { namespace os { inline void pin_worker_to_cpu( int core_id ) { 

    GROUP_AFFINITY groupAffinity = {0};
    groupAffinity.Mask  = (1ULL << (core_id % 64));
    groupAffinity.Group = (WORD)(core_id / 64);

    if( SetThreadGroupAffinity(GetCurrentThread(), &groupAffinity, NULL) ) 
      { return; }

    DWORD_PTR mask = (1ULL << core_id);
    SetThreadAffinityMask(GetCurrentThread(), mask);

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    struct PRIORITY{ enum FLAG {
         IDLE_PRIORITY    , LOW_PRIORITY     ,
         NORMAL_PRIORITY  , HIGH_PRIORITY    ,
         HIGHEST_PRIORITY , REALTIME_PRIORITY
    }; };

    inline int set_process_priority( int priority ){ DWORD n; switch( priority ) {
             case   PRIORITY::IDLE_PRIORITY:     n=IDLE_PRIORITY_CLASS;         break;
             case   PRIORITY::LOW_PRIORITY:      n=BELOW_NORMAL_PRIORITY_CLASS; break;
             case   PRIORITY::NORMAL_PRIORITY:   n=NORMAL_PRIORITY_CLASS;       break;
             case   PRIORITY::HIGH_PRIORITY:     n=ABOVE_NORMAL_PRIORITY_CLASS; break;
             case   PRIORITY::HIGHEST_PRIORITY:  n=HIGH_PRIORITY_CLASS;         break;
             case   PRIORITY::REALTIME_PRIORITY: n=REALTIME_PRIORITY_CLASS;     break; default: return -1;
        }    return SetPriorityClass( GetCurrentProcess(), n ) ? priority : -1;
    }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/