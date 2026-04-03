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

namespace nodepp { class invoke_t {
protected:

    using  NODE_CLB = function_t<int,any_t>;
    struct NODE {
        queue_t<NODE_CLB> queue;
    };  ptr_t<NODE> obj;

public:

   ~invoke_t() { if( obj.count() > 1 ){ return; } free(); }
    invoke_t() : obj( new NODE() ){}

    /*─······································································─*/

    ulong size () const noexcept { return obj->queue.size (); }
    void  clear() const noexcept { /*--*/ obj->queue.clear(); }
    void  free () const noexcept { /*--*/ obj->queue.clear(); }

    /*─······································································─*/

    int emit( string_t address, any_t value ) const noexcept {
        if( address.empty() ) /*-------------*/ { return -1; }
        auto mem = obj->queue.as( string::to_addr( address ) );
        if( mem == nullptr ) /*--------------*/ { return -1; }
        int c = mem->data.emit( value );
        if( c==-1 ){ off( address ); }
    return c; }

    int off( string_t address ) const noexcept {
        if( address.empty() ) /*-------------*/ { return -1; }
        auto mem = obj->queue.as( string::to_addr( address ) );
        if( mem == nullptr ) /*--------------*/ { return -1; }
        memset( address.get(), 0, address.size() );
        obj->queue.erase(mem); return 1;
    }

    string_t add( NODE_CLB callback ) const noexcept {
        auto clb = type::bind( callback );
        auto tsk = type::bind( (int) 0  );

        obj->queue.push ([=]( any_t value ){
            if( *tsk & TASK_STATE::USED ){ return -2; }
            *tsk |=  TASK_STATE::USED;
            int c = (*clb)( value );
            if( tsk.null() ) /*-------*/ { return -1; }
            *tsk &=~ TASK_STATE::USED;
        return c; });
        
        void* ID = obj->queue.last();
    return string::to_string( ID ); }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/