/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POPEN
#define NODEPP_POPEN

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS

    #include "fs.h"
    #include "worker.h"
    #include "initializer.h"
    #include "windows/popen.cpp"

    namespace nodepp { namespace popen {

        template< class... T > string_t await( const T&... args ){
            string_t out; auto pid = type::bind( popen_t( args... ) );
            pid->onData([&]( string_t chunk ){ out += chunk; });
            worker::await([&](){ return pid->next(); }); 
        return out; }

        template< class... T > popen_t async( const T&... args ){
            auto pid = type::bind( popen_t( args... ) );
            worker::add([=](){ return pid->next(); }); 
        return *pid; }

    }}

#elif _KERNEL_ == NODEPP_KERNEL_POSIX

    #include "fs.h"
    #include "initializer.h"
    #include "posix/popen.cpp"
    
    namespace nodepp { namespace popen {

        template< class... T > string_t await( const T&... args ){
            string_t out; auto pid = type::bind( popen_t( args... ) );
            pid->onData([&]( string_t chunk ){ out += chunk; });
            process::await([&](){ return pid->next(); }); 
        return out; }

        template< class... T > popen_t async( const T&... args ){
            auto pid = type::bind( popen_t( args... ) );
            process::foop([=](){ return pid->next(); }); 
        return *pid; }

    }}

#else
    #error "This OS Does not support popen.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif
