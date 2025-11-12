/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_STREAM
#define NODEPP_STREAM

/*────────────────────────────────────────────────────────────────────────────*/

#include "file.h"
#include "event.h"
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace stream {

    template< class T > 
    void unpipe( const T& input ){ input.stop(); input.onUnpipe.emit(); }
    
    /*─······································································─*/
    
    template< class... T >
    void duplex( const T&... inp ){ generator::stream::duplex arg;
         process::foop( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class... T >
    void until( const T&... inp ){ generator::stream::until arg;
         process::foop( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class... T >
    void pipe( const T&... inp ){ generator::stream::pipe arg;
         process::foop( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class... T >
    void line( const T&... inp ){ generator::stream::line arg;
         process::foop( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class T, class V >
    ulong await( const T& fa, const V& fb ){ 
        ulong out; /*-----------*/ generator::stream::pipe _read;
        while( fa.is_available() ){ out += fb.write( fa.read() ); } 
    return out; }
    
    template< class T >
    string_t await( const T& fp ){ 
        queue_t<string_t> out; generator::stream::pipe _read;
        while( fp.is_available() ){ out.push( fp.read() ); } 
    return array_t<string_t>( out.data() ).join(""); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif