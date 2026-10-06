/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT
#define NODEPP_EVENT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class... A > class event_t {
protected:

    using T = pair_t<function_t<int,A...>,ptr_t<task_t>>;

    struct NODE {
        queue_t<T>  que ; queue_t<void*> tmp; 
        uchar state=0x00;
    };  ptr_t<NODE> obj ;

    enum STATE : uchar {
         EV_STATE_UNKNOWN = 0b00000000,
         EV_STATE_KILL    = 0b10000000,
         EV_STATE_SKIP    = 0b00000001,
         EV_STATE_STOP    = 0b00000010,
         EV_STATE_USED    = 0b00000100,
         EV_STATE_CLOSED  = 0b00001000
    };

public:

    event_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    ptr_t<task_t> operator()( function_t<void,A...> cb ) const noexcept { return on(cb); }

    /*─······································································─*/

    ptr_t<task_t> add ( function_t<int,A...> cb ) const noexcept {
    ptr_t<task_t> task( 0UL, task_t() );

        obj->que.push({ [=]( A... args ){ return cb(args...); }, task });
        task->flag = TASK_STATE::OPEN;
        task->addr = obj->que.last() ;
        task->sign = &obj;

    return task; }

    ptr_t<task_t> once( function_t<void,A...> cb ) const noexcept {
    ptr_t<task_t> task( 0UL, task_t() );

        obj->que.push({ [=]( A... args ){ cb(args...); return -1; }, task });
        task->flag = TASK_STATE::OPEN;
        task->addr = obj->que.last() ;
        task->sign = &obj;

    return task; }

    ptr_t<task_t> on  ( function_t<void,A...> cb ) const noexcept {
    ptr_t<task_t> task( 0UL, task_t() );

        obj->que.push({ [=]( A... args ){ cb(args...); return  1; }, task });
        task->flag = TASK_STATE::OPEN;
        task->addr = obj->que.last() ;
        task->sign = &obj;

    return task; }

    /*─······································································─*/

    void off  ( ptr_t<task_t> address ) const noexcept { clear( address ); }
    void clear( ptr_t<task_t> address ) const noexcept {
        if( address.null() || empty() ) /*---*/ { return; }
        if( address->flag & TASK_STATE::CLOSED ){ return; }
        if( address->sign != &obj ) /*-------*/ { return; }
            auto node = obj->que.as( address->addr ); 
        if( node == nullptr ) /*-------------*/ { return; }
            address->flag = TASK_STATE::CLOSED;
        if( is_used() ){ obj->tmp.push ( address->addr ); }
        else /*-----*/ { obj->que.erase( node ); }
    }

    /*─······································································─*/

    void clear() const noexcept { 
        if( obj->state &  STATE::EV_STATE_USED )
          { obj->state |= STATE::EV_STATE_KILL; return; }  
            obj->state &=~STATE::EV_STATE_KILL;
        obj->que.clear();
    }

    /*─······································································─*/

    bool  empty() const noexcept { return obj->que.empty(); }
    ulong  size() const noexcept { return obj->que.size (); }

    /*─······································································─*/

    void emit( const A&... args ) const noexcept {
    if( is_paused() || is_used() || empty() ){ return; }

        obj->state|= STATE::EV_STATE_USED;

        auto x = obj->que.first(); while( x != nullptr ){
        auto y = x->next ; 

            if(( x->data.second->flag & STATE::EV_STATE_CLOSED ) ||
                 x->data.first.emit(args...)==-1
            )  { obj->que.erase( x ); }

        x=y; }

        obj->tmp.map([&]( void* item ){ obj->que.erase( obj->que.as(item) ); });
        /**/obj->state &=~ STATE::EV_STATE_USED; obj->tmp.clear();
        if( obj->state &   STATE::EV_STATE_KILL ){ clear(); }

    }

    /*─······································································─*/

    bool is_paused() const noexcept { 
        return obj->state & ( STATE::EV_STATE_SKIP |
        /*-----------------*/ STATE::EV_STATE_STOP );
    }

    bool is_used() const noexcept {
        return obj->state & STATE::EV_STATE_USED ;
    }

    void resume() const noexcept { 
        obj->state &=~ ( STATE::EV_STATE_STOP | 
        /*------------*/ STATE::EV_STATE_SKIP );
    }

    void stop() const noexcept { 
        obj->state |= STATE::EV_STATE_STOP;
    }

    void skip() const noexcept {
        obj->state |= STATE::EV_STATE_SKIP;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/