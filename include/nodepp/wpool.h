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
    using NODE_TASK= type::pair<ulong,void*>;
    using NODE_PAIR= type::pair<NODE_CLB,ref_t<task_t>>;

protected:

    struct DONE { probe_t t; ulong d; int c; };

    struct NODE {
           probe_t pool;  mutex_t mut; 
           queue_t<NODE_TASK> blocked;
           queue_t<void*>     normal;
           queue_t<NODE_PAIR> queue;
           ulong pool_size; 
    };     ptr_t<NODE> obj;

    /*─······································································─*/

    void* get_nearest_timeout( ulong time ) const noexcept {
    
        auto x = obj->blocked.last(); while( x!=nullptr ){
        if( time>=x->data.first ){ return x->next; }
        x = x->prev; }

    return obj->blocked.first(); }

    /*─······································································─*/

    inline int blocked_queue_next() const noexcept {
    
        auto x = obj->blocked.first(); do {
        if( x == nullptr ) /*----------*/ { return -1; }
        if( x->data.first>process::now() ){ return -1; }

        if( x->data.first < process::now() ){
            obj->normal .push ( x->data.second ); 
            obj->blocked.erase( x );
        }} while(0); return 1;

    }

    /*─······································································─*/

    inline int normal_queue_next() const noexcept {
    
        if( obj->normal.empty() ) /*-------*/ { return -1; } do {
        if( obj->normal.get()==nullptr ) /**/ { return -1; }
        if( obj->pool  .get()>obj->pool_size ){ return -1; }

        auto x = obj->normal.get(); auto y = obj->queue.as(x->data);
        auto o = obj->normal.get() == obj->normal.last() ? -1 : 1;
        auto self = type::bind(this); obj->normal.next();

        if( y->data.second->flag & TASK_STATE::USED   ){ return 1; }
        if( y->data.second->flag & TASK_STATE::CLOSED ){ 
            obj->queue .erase(y); 
            obj->normal.erase(x);
        return 1; } 

        y->data.second->flag |= TASK_STATE::USED;
        
        worker::add([=](){
        thread_local static DONE i = { self->obj->pool, 0, 0 };
        coStart; coYield(1);

            if( self->obj->mut.emit([=](){
            if( y->data.second->flag & TASK_STATE::CLOSED ){ return -1; }
            return 1; })==-1 ) /*-----------------------*/ { coEnd;     }

            do{ i.c=y->data.first(); auto z=coroutine::getno();
            if( i.c==1 && z.flag&coroutine::STATE::CO_STATE_DELAY )
              { i.d=z.delay;coStay(4); } switch(i.c) {
                case  1  :  coStay(5);   break;
                case -1  :  coStay(3);   break;
                case  0  :  coStay(1);   break;
            } } while(0) ;

            coEnd; coYield(5);

            self->obj->mut.lock([=](){
                y->data.second->flag &=~ TASK_STATE::USED;
            });

            coEnd; coYield(3);

            self->obj->mut.lock([=](){
                y->data.second->flag = TASK_STATE::CLOSED;
            });

            coEnd; coYield(4);

            self->obj->mut.lock([=](){

                y->data.second->flag &=~ TASK_STATE::USED;
                ulong wake_time = i.d + process::now();

                auto z = self->obj->blocked.as( self->get_nearest_timeout( wake_time ) );
                self->obj->blocked.insert( z, NODE_TASK( { wake_time, y } ));
                self->obj->normal .erase(x);

            }); coEnd;
            
        coStop }); return o; } while(0); return -1;

    }

public:

    wpool_t( ulong pool_size= MAX_POOL_SIZE ) noexcept : obj( new NODE() ) 
           { obj->pool_size = pool_size; }

    /*─······································································─*/

    void off( ptr_t<task_t> address ) const noexcept { clear( address ); }

    void clear( ptr_t<task_t> address ) const noexcept {
        if( address.null() ) /*--------------*/ { return; }
        if( address->sign != &obj ) /*-------*/ { return; }
        if( address->flag & TASK_STATE::CLOSED ){ return; }
            address->flag = TASK_STATE::CLOSED;
    }

    /*─······································································─*/

    int get_delay() const noexcept { 
        
        if(!obj->normal .empty() ){ return  0; }
        if( obj->blocked.empty() ){ return -1; }
        
        ulong wake = obj->blocked.first()->data.first;
        ulong now  = process::now();

    return wake<=now ? 0 : (int) min( wake - now, 60000UL ); }

    /*─······································································─*/

    void clear() const noexcept { 
        obj->queue  .clear(); 
        obj->normal .clear(); 
        obj->blocked.clear(); 
    }

    /*─······································································─*/

    void  set_pool_size( ulong pool_size ) const noexcept { obj->pool_size=pool_size; }

    ulong  blocked_size() const noexcept { return obj->blocked.size(); }

    ulong  running_size() const noexcept { return obj->normal.size (); }

    ulong          size() const noexcept { return obj->queue.size (); }

    bool          empty() const noexcept { return obj->queue.empty(); }

    ulong count_workers() const noexcept { return obj->pool.get(); }

    ulong get_pool_size() const noexcept { return obj->pool_size; }

    /*─······································································─*/

    inline int next() const noexcept { auto c = obj->mut.emit([&](){
        /*--*/ blocked_queue_next();
        return normal_queue_next ();
    }); return c<0 ? -1 : c; }

    /*─······································································─*/

    template< class T, class... V >
    ptr_t<task_t> add( T cb, const V&... args ) const noexcept {
    ptr_t<task_t> tsk( 0UL, task_t() ); auto clb = type::bind( cb );

        obj->queue .push({[=](){ return (*clb)( args... );}, tsk });
        obj->normal.push( obj->queue.last() ); 

        tsk->addr = obj->queue.last();
        tsk->flag = TASK_STATE::OPEN ;
        tsk->sign = &obj;

    return tsk; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif