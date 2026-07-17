/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WORKER
#define NODEPP_WORKER

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined( NODEPP_THREAD_SUPPORTED )
#error "This OS Does not support worker.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
    #include "mutex.h"
    #include "expected.h"
    #include "windows/worker.h"
#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #include "mutex.h"
    #include "expected.h"
    #include "posix/worker.h"
#else
    #error "This OS Does not support worker.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace worker { 

    template< class V, class... T >
    worker_t add( V cb, const T&... args ){ 
    worker_t wrk( cb, args... ); return wrk; }

    /*.........................................................................*/

    template< class V, class... T >
    void await( V cb, const T&... args ){
         ptr_t<bool> wait ( 0UL, true );
    worker::add([=](){
        if( cb( args... )>=0 ){ return  1; }
        *wait = false; /*----*/ return -1;
    }); while( *wait ){ process::next(); }}

    /*.........................................................................*/

    inline void parallel( void(*cb)(), int count ){
    for( int x=count-1; x-->0; ){ worker::add([=](){
         cb(); process::wait(); return -1;
    });} cb(); process::wait(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
