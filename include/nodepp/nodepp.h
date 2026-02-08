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
#include "evloop.h"
#include "env.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { array_t<string_t> args;

    template< class... T >
    void error( const T&... msg ){ throw except_t( msg... ); }

    /*─······································································─*/

    inline void start( int argc, char** args ){
        onSIGEXIT.once([=](){ process::exit(0); }); int i=0;
        do{ if(!regex::test(args[i],"^\\?") ) {
                process::args.push(args[i]);
        } else {
            for( auto &x: query::parse( args[i] ).data() )
               { env::set( x.first, x.second ); }
        }} while( i ++< argc - 1 ); signal::start(); 
    }

    /*─······································································─*/

    inline void stop(){ 
        while( !process::should_close() ){
            process::next( );
        }   process::exit(0);
    }

    inline void wait(){ process::stop(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

