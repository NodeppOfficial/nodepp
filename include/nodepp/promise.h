/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODE_PROMISE
#define NODE_PROMISE

/*────────────────────────────────────────────────────────────────────────────*/

#include "expected.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > using res_t = function_t<void,T>; }
namespace nodepp { template< class T > using rej_t = function_t<void,T>; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { enum PROMISE_STATE {
    UNDEFINED= 0b00000000,
    OPEN     = 0b00000001,
    PENDING  = 0b00000010,
    FINISHED = 0b00000100,
    CLOSED   = 0b00001000,
    RESOLVED = 0b00010000,
    REJECTED = 0b00100000,
    REJECTING= 0b01000000,
    RESOLVING= 0b10000000
}; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, class V > class promise_t {
private:

    using FINALLY  = event_t< >; /*-------------------*/
    using RESOLVE  = event_t<T>; /*-------------------*/
    using REJECT   = event_t<V>; /*-------------------*/
    using NODE_CLB = function_t<void,res_t<T>,rej_t<V>>;

protected:

    struct NODE {
        NODE_CLB node_clb;
        REJECT    rej_clb;
        RESOLVE   res_clb;
        FINALLY   fin_clb;
        uchar   state = 0;
    };  ptr_t<NODE> obj;

public:

    virtual ~promise_t() noexcept { if( obj.count()>1 ){ return; } invoke(); }

    promise_t( const NODE_CLB& cb ) noexcept : obj( new NODE() ) {
        obj->node_clb=cb; obj->state=PROMISE_STATE::OPEN;
    }

    promise_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    uchar get_state() const noexcept { return obj->state; }

    void close() const noexcept { off(); }

    void off() const noexcept {
        obj->state = PROMISE_STATE::CLOSED;
        obj->node_clb.free(); /*---------*/
    }

    /*─······································································─*/

    expected_t<T,V> await() const { do {

        if( obj->state== PROMISE_STATE::UNDEFINED ){ break; }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED    |
            /*--------*/ PROMISE_STATE::PENDING  )){ break; }

        obj->state|= PROMISE_STATE::PENDING; T res; V rej;
        auto self  = type::bind( this );

        self->obj->node_clb([&]( T value ){
            res = value; /*--------------------------*/
            self->obj->state = PROMISE_STATE::FINISHED;
            self->obj->state|= PROMISE_STATE::RESOLVED;
            self->obj->state|= PROMISE_STATE::CLOSED;
        },[&]( V value ){
            rej = value; /*--------------------------*/
            self->obj->state = PROMISE_STATE::FINISHED;
            self->obj->state|= PROMISE_STATE::REJECTED;
            self->obj->state|= PROMISE_STATE::CLOSED;
        });

        while((obj->state & PROMISE_STATE::PENDING)!= 0 )
             { process::next(); }

        if( obj->state & PROMISE_STATE::RESOLVED ){ return res; }
        if( obj->state & PROMISE_STATE::REJECTED ){ return rej; }

                throw except_t( "invalid result" );
    } while(0); throw except_t( "running in background" ); }

    /*─······································································─*/

    void resolve() const noexcept {

        if( obj->state== PROMISE_STATE::UNDEFINED ){ return; }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED   )){ return; }

        if((obj->state & PROMISE_STATE::PENDING)==0 ){ invoke(); }

        auto self = type::bind( this );

        process::await( coroutine::add( COROUTINE(){
        coBegin

        coWait(( self->obj->state & PROMISE_STATE::PENDING )!=0);

        coFinish }));

    }

    void invoke() const noexcept {

        if( obj->state== PROMISE_STATE::UNDEFINED ){ return; }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED    |
            /*--------*/ PROMISE_STATE::PENDING  )){ return; }

        obj->state|= PROMISE_STATE::PENDING;
        auto self  = type::bind( this );

        process::add([=](){

        self->obj->node_clb([=]( T res ){
            self->obj->res_clb.emit(res);
            self->obj->fin_clb.emit(   ); /*---------*/
            self->obj->state = PROMISE_STATE::FINISHED;
            self->obj->state|= PROMISE_STATE::RESOLVED;
            self->obj->state|= PROMISE_STATE::CLOSED;
        },[=]( V rej ){
            self->obj->rej_clb.emit(rej);
            self->obj->fin_clb.emit(   ); /*---------*/
            self->obj->state = PROMISE_STATE::FINISHED;
            self->obj->state|= PROMISE_STATE::REJECTED;
            self->obj->state|= PROMISE_STATE::CLOSED;
        }); 

        return -1; });

    }

    /*─······································································─*/

    template< class U >
    promise_t& then( const U cb ) noexcept {
        if( obj->state== PROMISE_STATE::UNDEFINED ){ return (*this); }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED   )){ return (*this); }

        obj->state |=PROMISE_STATE::RESOLVING;
        obj->res_clb.once(cb); return (*this);
    }

    template< class U >
    promise_t& fail( const U cb ) noexcept {
        if( obj->state== PROMISE_STATE::UNDEFINED ){ return (*this); }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED   )){ return (*this); }

        obj->state |=PROMISE_STATE::REJECTING;
        obj->rej_clb.once(cb); return (*this);
    }

    template< class U >
    promise_t& finally( const U cb ) noexcept {
        if( obj->state== PROMISE_STATE::UNDEFINED ){ return (*this); }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED   )){ return (*this); }

        obj->fin_clb.once(cb); return (*this); 
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace promise {

    template< class V >
    promise_t<V,except_t> all( const V& prom ) {
    return promise_t<V,except_t>([=]( res_t<V> res, rej_t<except_t>rej ){

        if( prom.empty() ){ rej( "iterator is empty" ); return; }

        process::add( coroutine::add( COROUTINE(){
        coBegin

            do{ for( auto &x: prom ){
            if( x.get_state() & PROMISE_STATE::PENDING ){ coGoto(0); }
            } } while(0);

            coNext;

            do{ for( auto &x: prom ){
            if( x.get_state() & PROMISE_STATE::REJECTED )
              { rej( except_t( "there are rejected promises" ) ); coEnd; }
            } } while(0);

            res( prom );

        coFinish
        }));

    }); }

    /*─······································································─*/

    template< class V >
    promise_t<V,except_t> any( const V& prom ) {
    return promise_t<V,except_t>([=]( res_t<V> res, rej_t<except_t>rej ){

        if( prom.empty() ){ rej( "iterator is empty" ); return; }

        process::add( coroutine::add( COROUTINE(){
        coBegin

            do{ for( auto &x: prom ){
            if( x.get_state() & PROMISE_STATE::PENDING ){ coGoto(0); }
            } } while(0);

            coNext;

            do{ for( auto &x: prom ){
            if( x.get_state() & PROMISE_STATE::RESOLVED )
              { res( x ); coEnd; }
            } } while(0);

            rej( except_t( "no fullfiled promises" ) );

        coFinish
        }));

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
