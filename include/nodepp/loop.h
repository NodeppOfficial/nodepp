/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_LOOP
#define NODEPP_LOOP

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class loop_t {
private:

    using NODE_CLB = function_t<int>;
    struct waiter { bool blk; bool out; };

protected:

    struct NODE {
        queue_t<NODE_CLB> queue;
    };  ptr_t<NODE> obj;

public:

    virtual ~loop_t() noexcept {}

    loop_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void clear() const noexcept { /*--*/ obj->queue.clear(); }

    ulong size() const noexcept { return obj->queue.size (); }

    bool empty() const noexcept { return obj->queue.empty(); }

    /*─······································································─*/

    int next() const noexcept {    
    auto x = obj->queue.get(); if( x==nullptr ){ return -1; }
    int  y = 0; bool z = x->next==nullptr;

        switch( (y=x->data()) ){
            case -1: obj->queue.erase(x); /*-----*/ break;
            case  1: obj->queue.next();   /*-----*/ break;
            default: /*----------------*/ return 0; break;
        } 
        
    return z ? -1 : y; }

    /*─······································································─*/

    template< class T, class... V >
    void* add( T cb, const V&... arg ) const noexcept {

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

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
