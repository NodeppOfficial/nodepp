/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_NODEPP
#define NODEPP_NODEPP

/*────────────────────────────────────────────────────────────────────────────*/

#include "import.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    array_t<string_t> args;

    /*─······································································─*/

    template< class... T >
    int  spawn( const T&... args ){ return ::system(args...); }

    template< class... T >
    void error( const T&... msg ){ _ERROR( msg... ); }

    void exit( int err=0 ){ ::exit(err); }

    void abort(){ ::abort(); }

    /*─······································································─*/

    void stop(){    while( !process::empty() )
        { onSIGNEXT.emit(); process::next(); }
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#include "query.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    void start(){ process::yield(); process::signal::start(); }

    void start( int argc, char** args ){
        int i=0; do {
            if(!regex::test(args[i],"^\\?") ) {
                process::args.push(args[i]);
            } else {
                for( auto &x: query::parse( args[i] ).data() )
                     process::env::set( x.first, x.second );
            }
        }   while( i ++< argc - 1 ); process::start();
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif