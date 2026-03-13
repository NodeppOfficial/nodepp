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
    
    template< class T, class V, class U >
    ptr_t<task_t> until( const T& fa, const V& fb, const U& val ){ generator::stream::until arg;
    return process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa, fb, val ); }
    
    template< class T, class U >
    ptr_t<task_t> until( const T& fa, const U& val ){ generator::stream::until arg;
    return process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa, val ); }

    /*─······································································─*/
    
    template< class T, class V >
    ptr_t<task_t> duplex( const T& fa, const V& fb ){ generator::stream::pipe arg;
           process::poll( arg, fb, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fb, fa ); 
    return process::poll( arg, fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa, fb ); }
    
    /*─······································································─*/
    
    template< class T, class V >
    ptr_t<task_t> line( const T& fa, const V& fb ){ generator::stream::line arg;
    return process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa, fb ); }
    
    template< class T >
    ptr_t<task_t> line( const T& fa ){ generator::stream::line arg;
    return process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa ); }
    
    /*─······································································─*/
    
    template< class T, class V >
    ptr_t<task_t> pipe( const T& fa, const V& fb ){ generator::stream::pipe arg;
    return process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa, fb ); }
    
    template< class T >
    ptr_t<task_t> pipe( const T& fa ){ generator::stream::pipe arg;
    return process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, arg, 0UL, fa ); }

    /*─······································································─*/
    
    template< class T >
    string_t await( const T& fp ){ 
        queue_t<string_t> out; generator::stream::pipe arg;
        fp.onData([&]( string_t data ){ out.push(data); });
    process::await( arg, fp ); return array_t<string_t>( out.data() ).join(""); }
    
    template< class T, class V >
    ulong await( const T& fa, const V& fb ){ 
        ulong out; /*-----------*/ generator::stream::pipe arg;
        fa.onData([&]( string_t data ){ out += data.size(); });
    process::await( arg, fa, fb ); return out; }
    
    /*─······································································─*/

    template< class T > 
    void unpipe( const T& input ){ input.stop(); input.onUnpipe.emit(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif