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

#if _KERNEL_ == NODEPP_KERNEL_WINDOWS
    
    #include "fs.h"
    #include "worker.h"
    #include "initializer.h"
    #include "windows/cluster.cpp"

    namespace nodepp { namespace cluster {

        template< class... T > cluster_t async( const T&... args ){
        auto pid = type::bind( cluster_t(args...) ); 
        if( process::is_parent() ) { 
            worker::add([=](){ return pid->next(); }); 
        } return *pid; }

        template< class... T > cluster_t add( const T&... args ){
        return async( args... ); }

        template< class... T > int await( const T&... args ){
        auto pid = type::bind( cluster_t(args...) );
        if( process::is_parent() ) { 
          return process::await([=](){ return pid->next(); }); 
        } return -1; }

        bool  is_child(){ return !process::env::get("CHILD").empty(); }

        bool is_parent(){ return  process::env::get("CHILD").empty(); }

    }}


#elif _KERNEL_ == NODEPP_KERNEL_POSIX

    #include "fs.h"
    #include "initializer.h"
    #include "posix/cluster.cpp"

    namespace nodepp { namespace cluster {

        template< class... T > cluster_t async( const T&... args ){
        auto pid = type::bind( cluster_t(args...) ); 
        if( process::is_parent() ) { 
            process::foop([=](){ return pid->next(); }); 
        } return *pid; }

        template< class... T > cluster_t add( const T&... args ){
        return async( args... ); }

        template< class... T > int await( const T&... args ){
        auto pid = type::bind( cluster_t(args...) );
        if( process::is_parent() ) { 
          return process::await([=](){ return pid->next(); }); 
        } return -1; }

        bool  is_child(){ return !process::env::get("CHILD").empty(); }

        bool is_parent(){ return  process::env::get("CHILD").empty(); }

    }}

#else
    #error "This OS Does not support cluster.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif