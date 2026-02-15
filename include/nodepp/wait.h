/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WAIT
#define NODEPP_WAIT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, class... A > class wait_t {
protected:

    using  DONE = function_t<bool,T,A...>;
    struct NODE {
        queue_t<DONE> que; void *addr =nullptr;
        /*--------------*/ int   state=0x00; 
    };  ptr_t  <NODE> obj;

    enum STATE {
         EV_STATE_UNKNOWN = 0b00000000,
         EV_STATE_SKIP    = 0b00000001,
         EV_STATE_STOP    = 0b00000010,
         EV_STATE_USED    = 0b00000100,
         EV_STATE_CLOSED  = 0b00001000
    };

public:

    wait_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    ptr_t<task_t> operator()( T val, function_t<void,T,A...> cb ) const noexcept { return on(val,cb); }

    /*─······································································─*/

    ptr_t<task_t> once( T val, function_t<void,A...> cb ) const noexcept {
    ptr_t<task_t> task( 0UL, task_t() ); auto clb= type::bind( cb );

        obj->que.push([=]( T arg, A... args ){
            if( task.null() || clb.null() ) /**/ { return false; }
            if( task->flag & TASK_STATE::CLOSED ){ return false; }
            if( val != arg ) /*---------------*/ { return true ; }
                task->flag = TASK_STATE::CLOSED; (*clb)(args...); 
        return false; }); 

        task->flag = TASK_STATE::OPEN;
        task->addr = obj->que.last();
        task->sign = &obj;

    return task; }

    ptr_t<task_t> add( T val, function_t<int,A...> cb ) const noexcept {
    ptr_t<task_t> task( 0UL, task_t() ); auto clb= type::bind( cb );

        obj->que.push([=]( T arg, A... args ){
            if( task.null() || clb.null() ) /**/ { return false; }
            if( task->flag & TASK_STATE::CLOSED ){ return false; }
            if( task->flag & TASK_STATE::USED   ){ return true ; }
            if( val != arg ) /*---------------*/ { return true ; }
                task->flag|= TASK_STATE::USED;   
            int c=(*clb)(args...); if(clb.null()){ return false; }
                task->flag&=~TASK_STATE::USED;
        return c==-1 ? false : true; });

        task->flag = TASK_STATE::OPEN;
        task->addr = obj->que.last();
        task->sign = &obj;

    return task; }

    ptr_t<task_t> on( T val, function_t<void,A...> cb ) const noexcept {
    ptr_t<task_t> task( 0UL, task_t() ); auto clb= type::bind( cb );

        obj->que.push([=]( T arg, A... args ){
            if( task.null() || clb.null() ) /**/ { return false; }
            if( task->flag & TASK_STATE::CLOSED ){ return false; }
            if( task->flag & TASK_STATE::USED   ){ return true ; }
            if( val != arg ) /*---------------*/ { return true ; }
                task->flag|= TASK_STATE::USED;   
            (*clb)(args...); if(clb.null()) /**/ { return false; }
                task->flag&=~TASK_STATE::USED;
        return true; });

        task->flag = TASK_STATE::OPEN;
        task->addr = obj->que.last();
        task->sign = &obj;

    return task; }

    /*─······································································─*/

    void off( ptr_t<task_t> address ) const noexcept {
        if( address.null() ) /*--------------*/ { return; }
        if( address->flag & TASK_STATE::CLOSED ){ return; }
        if( address->sign != &obj ) /*-------*/ { return; }
            address->flag = TASK_STATE::CLOSED;
        auto node = obj->que.as( address->addr ); 
        if( obj->addr == address->addr ){ obj->addr = node->next; }
        if( node != nullptr ) /*-----*/ { obj->que.erase( node ); }
    }

    /*─······································································─*/

    bool  empty() const noexcept { return obj->que.empty(); }
    ulong  size() const noexcept { return obj->que.size (); }
    void  clear() const noexcept { /*--*/ obj->que.clear(); }

    /*─······································································─*/

    void emit( const T& arg, const A&... args ) const noexcept {
        if( obj.null() || is_paused() || is_used() ){ return; }  

        obj->state |= STATE::EV_STATE_USED;
        auto x=obj->que.first();

        while( x!=nullptr && obj->que.is_valid(x) ){ obj->addr=x->next;
        if   ( !x->data.emit(arg,args...) ) /*--*/ { obj->que.erase(x); }
        x = obj->que.as( obj->addr ); }

        obj->state &=~ STATE::EV_STATE_USED;
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
