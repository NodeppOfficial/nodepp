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

namespace nodepp { template< class... T > class invoker_t {
protected:

    using NODE_CLB = function_t<int,T...>;
    /*-*/ handler_t<NODE_CLB> que;

public: invoker_t() {}

    bool  empty() const noexcept { return que.empty(); }
    ulong size () const noexcept { return que.size (); }
    void  clear() const noexcept { /*--*/ que.clear(); }
    void  free () const noexcept { /*--*/ que.clear(); }

    /*─······································································─*/

    bool is_valid( uchar_64 address ) const noexcept { return que.is_valid( address ); }
    int  off     ( uchar_64 address ) const noexcept { return que.remove  ( address ); }

    /*─······································································─*/

    int emit( uchar_64 address, const T&... arg ) const noexcept {
        auto mem = que.read( address );
        if( mem.null() ){ return -1; }
        int c = mem->emit( arg... );
        if( c==-1 )/*-*/{ off( address ); }
    return c; }

    /*─······································································─*/

    uchar_64 add( NODE_CLB clb ) const noexcept {
        auto tsk = ptr_t<int>( 0UL, 0x00 );
        auto mid = que.create();

        que.update( mid, [=]( const T&... args ){
        if( *tsk & TASK_STATE::USED ){ return -2; }
            *tsk|= TASK_STATE::USED; int c = clb( args... );
        if( tsk.null() ) /*-------*/ { return -1; }
            *tsk&=~TASK_STATE::USED;
        return c; });
        
    return mid; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/