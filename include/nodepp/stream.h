/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
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

    template< class T > void unpipe( const T& input )
        { input.stop(); input.onUnpipe.emit(); }
    
    /*─······································································─*/
    
    template< class... T >
    void duplex( const T&... inp ){ _stream_::duplex arg;
        process::poll::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    file_t pipe( const string_t& path, const string_t& mode ){
        auto inp = file_t( path, mode ); _stream_::pipe arg;
        process::poll::add( arg, inp );  return inp;
    }
    
    template< class... T >
    void pipe( const T&... inp ){ _stream_::pipe arg;
        process::poll::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    file_t line( const string_t& path, const string_t& mode ){
        auto inp = file_t( path, mode ); _stream_::line arg;
        process::poll::add( arg, inp );  return inp;
    }
    
    template< class... T >
    void line( const T&... inp ){ _stream_::line arg;
        process::poll::add( arg, inp... );
    }
    
    /*─······································································─*/
    
    template< class T, class V >
    ulong await( const T& fa, const V& fb ){ ulong result; _stream_::pipe _read;
        fa.onData([&]( string_t chunk ){ result += chunk.size(); });
        process::await( _read, fa, fb ); return result;
    }
    
    template< class T >
    string_t await( const T& fp ){ string_t result; _stream_::pipe _read;
        fp.onData([&]( string_t chunk ){ result += chunk; });
        process::await( _read, fp ); return result;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif