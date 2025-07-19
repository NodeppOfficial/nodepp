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
    
    template< class T >
    file_t until( const string_t& path, const string_t& mode, const T& search ){
        auto inp = file_t( path, mode ); generator::stream::until arg;
        process::add( arg, inp, search ); return inp;
    }
    
    template< class... T >
    void until( const T&... inp ){ generator::stream::until arg;
        process::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    file_t pipe( const string_t& path, const string_t& mode ){
        auto inp = file_t( path, mode ); generator::stream::pipe arg;
        process::add( arg, inp ); return inp;
    }
    
    template< class... T >
    void pipe( const T&... inp ){ generator::stream::pipe arg;
        process::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    file_t line( const string_t& path, const string_t& mode ){
        auto inp = file_t( path, mode ); generator::stream::line arg;
        process::add( arg, inp ); return inp;
    }
    
    template< class... T >
    void line( const T&... inp ){ generator::stream::line arg;
        process::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class... T >
    void duplex( const T&... inp ){ generator::stream::duplex arg;
        process::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class T, class V >
    ulong await( const T& fa, const V& fb ){ ulong out; generator::stream::pipe _read;
        fa.onData([&]( string_t chunk ){ out += chunk.size(); });
        process::await( _read, fa, fb ); return out;
    }
    
    template< class T >
    string_t await( const T& fp ){ string_t out; generator::stream::pipe _read;
        fp.onData([&]( string_t chunk ){ out += chunk; });
        process::await( _read, fp ); return out;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif