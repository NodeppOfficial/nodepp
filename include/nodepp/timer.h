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
    ptr_t<task_t> add ( V cb, ulong time, const T&... args ){
        auto clb = type::bind( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coDelay( time ); 
            
            if( (*clb)(args...)<0 ){ coEnd; } 

        coGoto(0); coFinish
        })); 
    }
    
    /*─······································································─*/

    inline void clear( ptr_t<task_t> address ){ process::clear( address ); }
    
    /*─······································································─*/

    template< class V, class... T >
    ptr_t<task_t> interval( V func, ulong time, const T&... args ){
    return timer::add([=]( T... args ){ func(args...); return 1; }, time, args... ); }

    template< class V, class... T >
    ptr_t<task_t> timeout ( V func, ulong time, const T&... args ){
    return timer::add([=]( T... args ){ func(args...); return -1; }, time, args... ); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace utimer {
    
    template< class V, class... T >
    ptr_t<task_t> add ( V cb, ulong time, const T&... args ){
        auto clb = type::bind( cb );
        return process::add( coroutine::add( COROUTINE(){
        coBegin coUDelay( time ); 
            
            if( (*clb)(args...)<0 ){ coEnd; } 

        coGoto(0); coFinish
        })); 
    }
    
    /*─······································································─*/

    inline void clear( ptr_t<task_t> address ){ process::clear( address ); }
    
    /*─······································································─*/
    
    template< class V, class... T >
    ptr_t<task_t> interval( V func, ulong time, const T&... args ){
    return utimer::add([=]( T... args ){ func(args...); return 1; }, time, args... ); }

    template< class V, class... T >
    ptr_t<task_t> timeout ( V func, ulong time, const T&... args ){
    return utimer::add([=]( T... args ){ func(args...); return -1; }, time, args... ); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
