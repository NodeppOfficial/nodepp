/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_INVOKE_DMA
#define NODEPP_INVOKE_DMA

/*────────────────────────────────────────────────────────────────────────────*/

#include "handler.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class... A > class invoker_t {
protected:

    using T = pair_t<function_t<int,A...>,ptr_t<task_t>>;
    handler_t<T> que;

public: invoker_t() {}

    bool  empty() const noexcept { return que.empty(); }
    ulong size () const noexcept { return que.size (); }
    void  clear() const noexcept { /*--*/ que.clear(); }
    void  free () const noexcept { /*--*/ que.clear(); }

    /*─······································································─*/

    bool is_valid( uchar_64 address ) const noexcept { return que.is_valid( address ); }
    int  off     ( uchar_64 address ) const noexcept { return que.remove  ( address ); }

    /*─······································································─*/

    handler_t<T> get_handler() const noexcept { return que; }

    /*─······································································─*/

    int emit( uchar_64 address, const A&... arg ) const noexcept {
    auto mem= que.read( address );

        if( mem.null() ) /*----------------------*/ { return -1; }
        if( mem->second->flag & TASK_STATE::USED   ){ return -2; }

        mem->second->flag |=  TASK_STATE::USED; 
        int c = mem->first.emit( arg... );
        mem->second->flag &=~ TASK_STATE::USED; 

        if( c==-1 )/*-*/{ off( address ); }

    return c; }

    /*─······································································─*/

    uchar_64 add( const function_t<int,A...>& clb ) const noexcept {
        auto tsk = ptr_t<task_t>( 0UL );
        auto mid = que.create();

        que.update( mid, { [=]( const A&... args ){ return clb(args...); }, tsk });

        tsk->flag = TASK_STATE::OPEN;
        tsk->addr = (void*) &que;
        tsk->sign = (void*) &que;
        
    return mid; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/