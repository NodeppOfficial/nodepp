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

#if   _KERNEL == NODEPP_KERNEL_WINDOWS

    #include "fs.h"
    #include "worker.h"
    #include "initializer.h"
    #include "windows/popen.cpp"

    namespace nodepp { namespace popen {

        template< class... T > string_t await( const T&... args ){
            string_t result; auto fp = popen_t( args... ); _stream_::pipe _read;
            fp.onData([&]( string_t chunk ){ result += chunk; });
            worker::await( _read, fp.std_output() ); return result;
        }

        template< class... T > popen_t async( const T&... args ){
        popen_t pid( args... ); worker::add([=](){ return pid.next(); }); 
        return pid; }

    }}

#elif _KERNEL == NODEPP_KERNEL_POSIX

    #include "fs.h"
    #include "initializer.h"
    #include "posix/popen.cpp"
    
    namespace nodepp { namespace popen {

        template< class... T > string_t await( const T&... args ){
            string_t result; auto fp = popen_t( args... ); _stream_::pipe _read;
            fp.onData([&]( string_t chunk ){ result += chunk; });
            process::await( _read, fp.std_output() ); 
        return result; }

        template< class... T > popen_t async( const T&... args ){
        popen_t pid( args... ); process::poll::add([=](){ return pid.next(); }); 
        return pid; }

    }}

#else
    #error "This OS Does not support popen.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif
