/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_NODEPP
#define NODEPP_NODEPP

/*────────────────────────────────────────────────────────────────────────────*/

#include "import.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { array_t<string_t> args;

    /*─······································································─*/

    template< class... T >
    void error( const T&... msg ){ throw except_t( msg... ); }

    void exit( int err=0 ){ _EXIT_= true; ::exit(err); }

    /*─······································································─*/

    void start(){ process::yield(); process::signal::start(); }

    void start( int argc, char** args ){
        int i=0; onSIGEXIT.once([=](){ process::exit(1); }); do {
            if(!regex::test(args[i],"^\\?") ) {
                process::args.push(args[i]);
            } else {
                for( auto &x: query::parse( args[i] ).data() )
                   { process::env::set( x.first, x.second ); }
            }
        }   while( i ++< argc - 1 ); process::start();
    }

    /*─······································································─*/

    void stop(){ 
        while(!process::should_close() )
             { process::next(); }
        process::exit();
    }

    /*─······································································─*/

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
