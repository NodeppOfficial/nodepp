/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_CLUSTER
#define NODEPP_CLUSTER

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL == NODEPP_KERNEL_WINDOWS
    #include "fs.h"
    #include "initializer.h"
    #include "windows/cluster.cpp"
#elif _KERNEL == NODEPP_KERNEL_POSIX
    #include "fs.h"
    #include "initializer.h"
    #include "posix/cluster.cpp"
#else
    #error "This OS does not support cluster.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace cluster {

    template< class... T > cluster_t add( const T&... args ){
    cluster_t pid(args...); if( process::is_parent() ) { 
       process::poll::add([=](){ return pid.next(); }); 
    }  return pid; }

    /*─······································································─*/

    bool  is_child(){ return !process::env::get("CHILD").empty(); }

    bool is_parent(){ return  process::env::get("CHILD").empty(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif