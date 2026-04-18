/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_DMA_INVOKE
#define NODEPP_DMA_INVOKE

/*────────────────────────────────────────────────────────────────────────────*/

#include "any.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class... T > class invoke_t {
protected:

    using   NODE_CLB = function_t<int,T...>;
    queue_t<NODE_CLB> queue;

public: invoke_t() {}

    /*─······································································─*/

    const queue_t<NODE_CLB>& get_queue() const noexcept { return queue; }

    /*─······································································─*/

    bool  empty() const noexcept { return queue.empty(); }
    ulong size () const noexcept { return queue.size (); }
    void  clear() const noexcept { /*--*/ queue.clear(); }
    void  free () const noexcept { /*--*/ queue.clear(); }

    /*─······································································─*/

    int emit( void* address, const T&... arg ) const noexcept {
        if( address == nullptr || empty() ){ return -1; }
        auto mem = queue .as ( address );
        int c = mem->data.emit( arg... );
        if( c==-1 ) { off( address ); }
    return c; }

    /*─······································································─*/

    int off( void* address ) const noexcept {
        if( address == nullptr || empty() ){ return -1; }
        auto mem = queue.as( address );
        if( mem == nullptr ) /*---------*/ { return -1; }
    queue.erase(mem); return 1; }

    /*─······································································─*/

    void* add( NODE_CLB callback ) const noexcept {
        auto clb = type::bind( callback );
        auto tsk = type::bind( (int) 0  );

        queue.push ([=]( const T&... args ){
        if( *tsk & TASK_STATE::USED ){ return -2; }
            *tsk |=  TASK_STATE::USED;
            int c = (*clb)( args... );
        if( tsk.null() ) /*-------*/ { return -1; }
            *tsk &=~ TASK_STATE::USED;
        return c; });
        
    return queue.last(); }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class emitter_t : public invoke_t<any_t> {
protected: invoke_t obj; public: emitter_t() : invoke_t() {}

    /*─······································································─*/

    ulong size () const noexcept { return obj.size (); }
    void  clear() const noexcept { /*--*/ obj.clear(); }
    void  free () const noexcept { /*--*/ obj.clear(); }

    /*─······································································─*/

    int emit( string_t address, any_t value ) const noexcept {
        return obj.emit( string::to_addr( address ), value );
    }

    string_t add( NODE_CLB callback ) const noexcept {
        return string::to_string( obj.add( callback ) );
    }

    int off( string_t address ) const noexcept {
        return obj.off( string::to_addr( address ) );
    }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/