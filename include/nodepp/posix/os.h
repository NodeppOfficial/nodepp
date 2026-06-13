/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_OS
#define NODEPP_POSIX_OS

/*────────────────────────────────────────────────────────────────────────────*/

#include <sys/resource.h>
#include <unistd.h>
#include <cerrno>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {
    
    inline string_t hostname(){ 
        char buff /*----------*/ [NODEPP_UNBFF_SIZE]; 
        return ::gethostname(buff,NODEPP_UNBFF_SIZE)==0 ? buff : nullptr; }
    
    /*─······································································─*/

    inline string_t cwd(){
        char buff /*-----*/ [NODEPP_UNBFF_SIZE]; 
        return ::getcwd(buff,NODEPP_UNBFF_SIZE)==nullptr ? nullptr : buff; }
    
    /*─······································································─*/

    inline uint cpus(){ return ::sysconf( _SC_NPROCESSORS_ONLN ); }
    
    /*─······································································─*/

    inline int exec( string_t cmd ){ return ::system( cmd.get() ); }

    inline int call( string_t cmd ){ return ::system( cmd.get() ); }
    
    /*─······································································─*/

    inline string_t user(){ return ::getlogin(); }
    
    /*─······································································─*/

    inline string_t tmp(){ return "/tmp"; }

    /*─······································································─*/

    inline uint pid(){ return ::getpid(); }

    /*─······································································─*/

    inline uint error(){ return errno; }

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

namespace nodepp { namespace os {

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

namespace nodepp { namespace os {
    
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

#if ( _OS_ == NODEPP_OS_LINUX )
    #include <sched.h>
    namespace nodepp { namespace os { inline void pin_worker_to_cpu( int core_id ){ 
        cpu_set_t mask; CPU_ZERO(&mask); CPU_SET(core_id, &mask);
        sched_setaffinity(0, sizeof(mask), &mask);
    }}}

#elif ( _OS_ == NODEPP_OS_FRBSD )
    #include <sys/param.h>
    #include <sys/cpuset.h>
    #include <pthread_np.h>
    namespace nodepp { namespace os { inline void pin_worker_to_cpu( int core_id ){ 
        cpuset_t mask; CPU_ZERO(&mask); CPU_SET(core_id, &mask);
        cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, sizeof(mask), &mask);
    }}}

#elif ( _OS_ == NODEPP_OS_APPLE )
    #include <mach/mach.h>
    #include <mach/thread_policy.h>
    namespace nodepp { namespace os { inline void pin_worker_to_cpu( int core_id ){ 
        thread_affinity_policy_data_t policy = { core_id };
        thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
    }}}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/