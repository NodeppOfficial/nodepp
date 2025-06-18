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
    #include "initializer.h"
    #include "windows/popen.cpp"
#elif _KERNEL == NODEPP_KERNEL_POSIX
    #include "fs.h"
    #include "initializer.h"
    #include "posix/popen.cpp"
#else
    #error "This OS Does not support popen.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace popen {

    popen_t async( const string_t& path, const initializer_t<string_t>& args ){
    popen_t pid( path, args ); process::poll::add([=](){ return pid.next(); }); 
    return pid; }

    popen_t async( const string_t& path ){ return async( path, { path } ); }

    /*─······································································─*/

    string_t await( const string_t& path, const initializer_t<string_t>& args ){
        string_t result; auto fp = popen_t( path, args ); _stream_::pipe _read;
        fp.onData([&]( string_t chunk ){ result += chunk; });
        process::await( _read, fp.std_output() ); return result;
    }

    string_t await( const string_t& path ){
      return await( path, { path } );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
