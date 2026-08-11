/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TIMER
#define NODEPP_TIMER

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace timer {
    
    template< class V, class... T >
    ptr_t<task_t> add ( const V& cb, ulong time, const T&... args ){
        auto clb = function_t<int,T...>( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coDelay( time ); coSet(0); return clb(args...); coFinish
    })); }
    
    template< class V, class... T >
    ptr_t<task_t> timeout ( const V& cb, ulong time, const T&... args ){
        auto clb = function_t<void,T...>( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coDelay( time ); clb(args...); coFinish
    })); }
    
    template< class V, class... T >
    ptr_t<task_t> interval ( const V& cb, ulong time, const T&... args ){
        auto clb = function_t<void,T...>( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coDelay( time ); clb(args...); coGoto(0); coFinish
    })); }
    
    /*─······································································─*/

    inline void clear( ptr_t<task_t> address ){ process::clear( address ); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace utimer {
    
    template< class V, class... T >
    ptr_t<task_t> add ( const V& cb, ulong time, const T&... args ){
        auto clb = function_t<int,T...>( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coUDelay( time ); coSet(0); return clb(args...); coFinish
    })); }
    
    template< class V, class... T >
    ptr_t<task_t> timeout ( const V& cb, ulong time, const T&... args ){
        auto clb = function_t<void,T...>( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coUDelay( time ); clb(args...); coFinish
    })); }
    
    template< class V, class... T >
    ptr_t<task_t> interval ( const V& cb, ulong time, const T&... args ){
        auto clb = function_t<void,T...>( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coUDelay( time ); clb(args...); coGoto(0); coFinish
    })); }
    
    /*─······································································─*/

    inline void clear( ptr_t<task_t> address ){ process::clear( address ); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/