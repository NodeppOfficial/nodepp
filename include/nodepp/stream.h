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
#include "promise.h"
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
    return process::add( arg, fa );
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
    promise_t< string_t, except_t > resolve( const T& fa ){
    return promise_t< string_t, except_t > ([=](
        res_t<string_t> res, rej_t<except_t> rej
    ){

        if( fa.is_closed() ){ rej( except_t( "closed file" ) ); return; }

        ptr_t<string_t> bff ( 0UL );

        fa.onData ([=]( string_t chunk ){ *bff += chunk; });
        fa.onDrain([=](){ res( *bff ); });
        stream::pipe( fa );

    }); }

    template< class T, class V >
    promise_t< string_t, except_t > resolve( const T& fa, const V& fb ){
    return promise_t< string_t, except_t > ([=](
        res_t<string_t> res, rej_t<except_t> rej
    ){

        if( fa.is_closed() ){ rej( except_t( "closed file" ) ); return; }
        if( fb.is_closed() ){ rej( except_t( "closed file" ) ); return; }

        ptr_t<string_t> bff ( 0UL );

        fa.onData ([=]( string_t chunk ){ *bff += chunk; });
        fa.onDrain([=](){ res( *bff ); });
        stream::pipe( fa, fb );

    }); }

    /*─······································································─*/
    
    template< class T, class V >
    expected_t<string_t,except_t> await( const T& fa, const V& fb ){ 
        return stream::resolve( fa, fb ).await();
    }
    
    template< class T >
    expected_t<string_t,except_t> await( const T& fa ){ 
        return stream::resolve( fa ).await(); 
    }
    
    /*─······································································─*/

    template< class T > 
    void unpipe( const T& input ){ input.stop(); input.onUnpipe.emit(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif