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

        if( fa.is_closed() ){ rej( except_t( "closed stream" ) ); return; }

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

        if( fa.is_closed() ){ rej( except_t( "closed stream" ) ); return; }
        if( fb.is_closed() ){ rej( except_t( "closed stream" ) ); return; }

        ptr_t<string_t> bff ( 0UL );

        fa.onData ([=]( string_t chunk ){ *bff += chunk; });
        fa.onDrain([=](){ res( *bff ); });
        stream::pipe( fa, fb );

    }); }

    /*─······································································─*/

    template< class T >
    promise_t<T,except_t> readable ( const T& fa, ulong timeout ) {
    return promise_t< T, except_t > ([=](
        res_t<T> res, rej_t<except_t> rej
    ){

        ptr_t<ulong> time ( 0UL, timeout + process::now() );

        process::poll( fa, POLL_STATE::READ | POLL_STATE::EDGE, [=]( const T& fa ){

            int c=0; if( *time!=0 && process::now()<*time ){ rej("timeout"); return -1; }

            while((c=fa._read( fa.get_buffer_data(), fa.get_buffer_size() ))==-2){ 
            if   ( fa.is_waiting() ){ return 0; }  return  1; }
            if   ( c<=0 ){ rej( "stream closed" ); return -1; } 
            fa.set_borrow( string_t( fa.get_buffer_data(),c ) ); res( fa );

        return -1; }, 0UL, fa );

    }); }

    template< class T >
    promise_t<T,except_t> writable ( const T& fa, string_t message, ulong timeout ) {
    return promise_t< T, except_t > ([=](
        res_t<T> res, rej_t<except_t> rej
    ){

        ptr_t<ulong> time ( 0UL, timeout + process::now() );
        ptr_t<ulong> by   ( 0UL, 0UL );

        process::poll( fa, POLL_STATE::WRITE | POLL_STATE::EDGE, [=]( const T& fa ){

            int c=0; if( *time!=0 && process::now()<*time ){ rej("timeout"); return -1; }

            while((c=fa._write_ ( message.get(), message.size(), &by ))==-2){ 
            if   ( fa.is_waiting() ){ return 0; }  return  1; }
            if   ( c<=0 ){ rej( "stream closed" ); return -1; } res( fa );

        return -1; }, 0UL, fa );

    }); }

    /*─······································································─*/
    
    template< class T, class V >
    expected_t<string_t,except_t> await( const T& fa, const V& fb ){ 
        
        if( fa.is_closed() || fb.is_closed() ){ return except_t( "closed stream" ); }
        string_t bff; generator::stream::line task;

        fa.onData ([&]( string_t chunk ){ bff += chunk; });
        while( task( fa, fb )==1 ){ process::next(); }

        return bff;

    }
    
    template< class T >
    expected_t<string_t,except_t> await( const T& fa ){ 
        
        if( fa.is_closed() ){ return except_t( "closed stream" ); }
        string_t bff; generator::stream::line task;

        fa.onData ([&]( string_t chunk ){ bff += chunk; });
        while( task( fa )==1 ){ process::next(); }

        return bff;

    }
    
    /*─······································································─*/

    template< class T > 
    void unpipe( const T& input ){ input.stop(); input.onUnpipe.emit(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif