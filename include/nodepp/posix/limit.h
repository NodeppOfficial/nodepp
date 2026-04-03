/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_LIMIT
#define NODEPP_POSIX_LIMIT

/*────────────────────────────────────────────────────────────────────────────*/

#include <sys/resource.h>
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

    inline uint get_hard_fileno() { struct rlimit limit;
        if( getrlimit( RLIMIT_NOFILE, &limit )==0 ) 
          { return limit.rlim_max; } return 1024;
    }

    inline uint get_soft_fileno() { struct rlimit limit;
        if( getrlimit( RLIMIT_NOFILE, &limit )==0 ) 
          { return limit.rlim_cur; } return 1024;
    }

    inline int set_hard_fileno( uint value ) {
        struct rlimit limit;
        limit.rlim_max = value;
        limit.rlim_cur = get_soft_fileno();
        return setrlimit( RLIMIT_NOFILE, &limit );
    }

    inline int set_soft_fileno( uint value ) {
        struct rlimit limit;
        limit.rlim_cur = value;
        limit.rlim_max = get_hard_fileno();
        return setrlimit( RLIMIT_NOFILE, &limit );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {

    inline uint get_hard_thread_pool() { struct rlimit limit;
        if( getrlimit( RLIMIT_NPROC, &limit )==0 ) 
          { return limit.rlim_max; } return 1024;
    }

    inline uint get_soft_thread_pool() { struct rlimit limit;
        if( getrlimit( RLIMIT_NPROC, &limit )==0 ) 
          { return limit.rlim_cur; } return 1024;
    }

    inline int set_hard_soft_thread_pool( uint value ) {
        struct rlimit limit;
        limit.rlim_max = value;
        limit.rlim_cur = get_soft_thread_pool();
        return setrlimit( RLIMIT_NPROC, &limit );
    }

    inline int set_soft_thread_pool( uint value ) {
        struct rlimit limit;
        limit.rlim_cur = value;
        limit.rlim_max = get_hard_thread_pool();
        return setrlimit( RLIMIT_NPROC, &limit );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {
    
    struct PRIORITY{ enum FLAG {
         IDLE_PRIORITY    , LOW_PRIORITY     ,
         NORMAL_PRIORITY  , HIGH_PRIORITY    ,
         HIGHEST_PRIORITY , REALTIME_PRIORITY
    }; };
    
    inline int set_process_priority( int priority ){ int n; switch( priority ) {
        case   PRIORITY::IDLE_PRIORITY:     n =  19; break;
        case   PRIORITY::LOW_PRIORITY:      n =  5 ; break;
        case   PRIORITY::NORMAL_PRIORITY:   n =  0 ; break;
        case   PRIORITY::HIGH_PRIORITY:     n = -5 ; break;
        case   PRIORITY::HIGHEST_PRIORITY:  n = -15; break;
        case   PRIORITY::REALTIME_PRIORITY: n = -20; break; default: return -1;
    }   return nice(n)!=-1 ? priority : -1; }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/