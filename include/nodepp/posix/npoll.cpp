/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { enum POLL_STATE {
    READ = 1, WRITE = 2, DUPLEX = 3
};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class poll_t {
private:

    using NODE_CLB = function_t<int>;
    struct waiter { bool blk; bool out; };

protected:

    struct NODE {
        queue_t<NODE_CLB> queue;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    template< class T, class... V >
    void* push( T cb, const V&... arg ) const noexcept {

        ptr_t<waiter> tsk = new waiter();
        auto clb=type::bind(cb); tsk->blk=0; tsk->out=1; 

        obj->queue.push([=](){
            if( tsk->out==0 ){ return -1; }
            if( tsk->blk==1 ){ return  1; } 
                tsk->blk =1; int rs=(*clb)( arg... );
            if( clb.null()  ){ return -1; }  
                tsk->blk =0;   return !tsk->out?-1:rs;
        }); 
        
        return (void*) &tsk->out;
    }

public:

    virtual ~poll_t() noexcept {}
    /*----*/ poll_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void clear() const noexcept { /*--*/ obj->queue.clear(); }

    ulong size() const noexcept { return obj->queue.size (); }

    bool empty() const noexcept { return obj->queue.empty(); }

    /*─······································································─*/

    inline int next() const noexcept {
    if( empty() )   { return -1; } auto x = obj->queue.get();
    if( x==nullptr ){ return -1; } bool y = x->next==nullptr;
    /*--------------------------*/ int  c = 0;
        
        switch( c=x->data() ){
            case -1: obj->queue.erase(x); break;
            case  1: obj->queue.next();   break;
            default: /*----------------*/ break;
        }

    return y ? -1 : c; }

    /*─······································································─*/

    template< class T, class U, class... W >
    void* add( T, uchar, U cb, const W&... args ) noexcept {
        return push( cb, args... );
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/