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
    #include "worker.h"
    #include "initializer.h"
    #include "windows/cluster.cpp"
#elif _KERNEL == NODEPP_KERNEL_POSIX
    #include "fs.h"
    #include "initializer.h"
    #include "posix/cluster.cpp"
#else
    #error "This OS Does not support cluster.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if defined( NODEPP_POPEN ) && _KERNER == NODEPP_KERNEL_WINDOWS
namespace nodepp { namespace cluster {

    template< class... T > cluster_t async( const T&... args ){
    cluster_t pid(args...); if( process::is_parent() ) { 
       worker::add([=](){ return pid.next(); }); 
    }  return pid; }

    template< class... T > int await( const T&... args ){
    cluster_t pid(args...); if( process::is_parent() ) { 
       return worker::await([=](){ return pid.next(); }); 
    }  return -1; }

    template< class... T > cluster_t add( const T&... args ){
    return async( args... ); }

    /*─······································································─*/

    bool  is_child(){ return !process::env::get("CHILD").empty(); }

    bool is_parent(){ return  process::env::get("CHILD").empty(); }

}}

#else 

namespace nodepp { namespace cluster {

    template< class... T > cluster_t async( const T&... args ){
    cluster_t pid(args...); if( process::is_parent() ) { 
       process::poll::add([=](){ return pid.next(); }); 
    }  return pid; }

    template< class... T > int await( const T&... args ){
    cluster_t pid(args...); if( process::is_parent() ) { 
       return process::await([=](){ return pid.next(); }); 
    }  return -1; }

    template< class... T > cluster_t add( const T&... args ){
    return async( args... ); }

    /*─······································································─*/

    bool  is_child(){ return !process::env::get("CHILD").empty(); }

    bool is_parent(){ return  process::env::get("CHILD").empty(); }

}}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif