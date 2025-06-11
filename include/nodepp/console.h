/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_CONSOLE
#define NODEPP_CONSOLE

/*────────────────────────────────────────────────────────────────────────────*/

#include "conio.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace console {

    template< class... T >
    int err( const T&... args ){ return conio::err(args...,"\n"); }

    template< class... T >
    int log( const T&... args ){ return conio::log(args...,"\n"); }

    template< class... T >
    int scan( const T&... args ){ return conio::scan( args... ); }

    template< class... T >
    int pout( const T&... args ){ return conio::log( args... ); }

    void wait(){ char x; conio::scan("%c",&x); }

    void clear(){ conio::clear(); }
    
    /*─······································································─*/

    template< class... T >
    int warning( const T&... args ){ 
        conio::warn("WARNING: ");
        return log( args... ); 
    }

    template< class... T >
    int success( const T&... args ){ 
        conio::done("SUCCESS: ");
        return log( args... );  
    }

    template< class... T >
    int error( const T&... args ){ 
        conio::error("ERROR: "); 
        return log( args... ); 
    }

    template< class... T >
    int done( const T&... args ){ 
        conio::done("DONE: "); 
        return log( args... ); 
    }

    template< class... T >
    int info( const T&... args ){ 
        conio::info("INFO: "); 
        return log( args... ); 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif