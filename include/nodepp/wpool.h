/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WORKER_POOL
#define NODEPP_WORKER_POOL

/*────────────────────────────────────────────────────────────────────────────*/

#include "mutex.h"
#include "worker.h"
#include "atomic.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class wpool_t : public generator_t {
private:

    using NODE_CLB = function_t<int>;
    struct waiter { bool blk; bool out; };
    using NODE_PAIR= type::pair<NODE_CLB,ref_t<waiter>>;

protected:

    struct NODE {
        atomic_t<ulong> pool=0; mutex_t mut;
        ulong min_stamp=0, pool_size=0 , dif=0; 
        queue_t<NODE_PAIR> /*---------*/ queue;
        queue_t<void*> /*-------------*/ normal;
        queue_t<type::pair<ulong,void*>> blocked;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    inline int blocked_queue_next() const noexcept {
    
        if( obj->blocked.empty() ) /*---*/ { return -1; } do {
        if( obj->blocked.get() == nullptr ){ return -1; }
        if( obj->min_stamp>process::now() ){ return -1; }

        auto x = obj->blocked.get();

        if( x->data.first < process::now() ){
            obj->normal .push ( x->data.second ); 
            obj->blocked.erase( x );
        if(!obj->blocked.empty()) {
            obj->min_stamp = obj->blocked.first()->data.first;
        }} else {
            obj->blocked.next();
        }

        } while(0); return 1;

    }

    /*─······································································─*/

    inline int normal_queue_next() const noexcept {
    
        if( obj->normal.empty() ) /*-------*/ { return -1; } do {
        if( obj->normal.get()==nullptr ) /**/ { return -1; }
        if( obj->pool  .get()>obj->pool_size ){ return -1; }

        auto x = obj->normal.get(); auto y = obj->queue.as(x->data);
        auto o = obj->normal.get() == obj->normal.last();
        auto self = type::bind(this); obj->normal.next();

        if( y->data.second->blk==1 ){ return o; }
        if( y->data.second->out==0 ){ 
            obj->queue .erase(y); 
            obj->normal.erase(x);
        return o; } 
        
        y->data.second->blk =1;

        worker::add( [=](){
            thread_local static int   c=0;
            thread_local static ulong d=0;
        coStart; ++self->obj->pool; coYield(1);

            coWait((c=self->obj->mut.emit([=](){
                if( y->data.second->out== 0 ){ return -1; }
            return 1; }))==-2 ); if ( c==-1 ){ coStay(6); }

            do{ c=y->data.first(); auto z=coroutine::getno();
            if( c==1 && z.flag&coroutine::STATE::CO_STATE_DELAY )
              { d=z.delay; coStay(4); } switch(c) {
                case  1 :  coStay(5);   break;
                case  0 :  coStay(1);   break;
                case -1 :  coStay(3);   break;
            } } while(0);

            coYield(5);

            coWait((c=self->obj->mut.emit([=](){
                y->data.second->blk = 0;
                self->obj->dif      = 0;
            return -1; }))==-2 ); coStay(6);

            coYield(3);

            coWait((c=self->obj->mut.emit([=](){
                y->data.second->out = 0;
                y->data.second->blk = 0;
            return -1; }))==-2 ); if( c==-1 ){ coStay(6); } 

            coYield(4);

            coWait((c=self->obj->mut.emit([=](){

                ulong wake_time=d+process::now ();
                y->data.second->blk=0; 

                self->obj->blocked.push ({ wake_time, y });
                self->obj->normal .erase(x); 

                if( self->obj->min_stamp > wake_time ||  
                    self->obj->blocked.size()==1
                ) { 
                    self->obj->min_stamp=wake_time;
                    self->obj->dif      =d;
                }

            return -1; }))==-2 ); if( c==-1 ){ coStay(6); }
            
        coYield(6); --self->obj->pool; coStop });

        return o ? -1 : 1; } while(0); return -1;

    }

public:

    wpool_t( ulong pool_size= MAX_POOL_SIZE ) noexcept : obj( new NODE() ) 
           { obj->pool_size = pool_size; }

    virtual ~wpool_t() noexcept { /*--*/ }

    /*─······································································─*/

    void clear() const noexcept { /*--*/ obj->queue.clear(); obj->normal.clear(); obj->blocked.clear(); }

    ulong size() const noexcept { return obj->queue.size (); }

    bool empty() const noexcept { return obj->queue.empty(); }

    /*─······································································─*/

    int       get_delay() const noexcept { return empty()?-1:type::cast<int>(obj->dif); }

    void  set_pool_size( ulong pool_size ) const noexcept { obj->pool_size=pool_size; }

    ulong count_workers() const noexcept { return obj->pool.get(); }

    ulong get_pool_size() const noexcept { return obj->pool_size; }

    /*─······································································─*/

    inline int next() const noexcept { auto c = obj->mut.emit([&](){
        /*--*/ blocked_queue_next();
        return normal_queue_next ();
    }); return c<0 ? -1 : c; }

    /*─······································································─*/

    template< class T, class... V >
    inline void* add( T cb, const V&... args ) const noexcept {
        ptr_t<waiter> tsk = new waiter(); tsk->blk=0; tsk->out=1; 
        auto clb = type::bind( cb );
        obj->queue .push({ [=](){ return (*clb)( args... ); }, tsk });
        obj->normal.push( obj->queue.last() ); return (void*)&tsk->out;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif