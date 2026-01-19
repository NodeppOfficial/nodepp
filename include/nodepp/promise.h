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

namespace nodepp { struct PROMISE_STATE { enum TYPE {
    UNDEFINED= 0b00000000,
    OPEN     = 0b00000001,
    PENDING  = 0b00000010,
    FINISHED = 0b00000100,
    CLOSED   = 0b00001000,
    RESOLVED = 0b00010000,
    REJECTED = 0b00100000,
    REJECTING= 0b01000000,
    RESOLVING= 0b10000000
};}; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, class V > class promise_t {
private:

    using FINALLY  = event_t< >; /*-------------------*/
    using RESOLVE  = event_t<T>; /*-------------------*/
    using REJECT   = event_t<V>; /*-------------------*/
    using NODE_CLB = function_t<void,res_t<T>,rej_t<V>>;

protected:

    struct NODE {
        ptr_t<task_t> tsk;
        any_t /*-*/ value;
        NODE_CLB node_clb;
        REJECT    rej_clb;
        RESOLVE   res_clb;
        FINALLY   fin_clb;
        uchar     state=0;
    };  ptr_t<NODE> obj;

protected:

    void invoke() const {

        if( obj->state== PROMISE_STATE::UNDEFINED ){ return; }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED    |
            /*--------*/ PROMISE_STATE::PENDING  )){ return; }

        obj->state|= PROMISE_STATE::PENDING;
        auto self  = type::bind( this );

        self->obj->node_clb([=]( T value ){
            self->obj->value = value; /*-------------*/
            self->obj->res_clb.emit(value); /*-------*/
            self->obj->fin_clb.emit(/*-*/); /*-------*/
            self->obj->state = PROMISE_STATE::FINISHED;
            self->obj->state|= PROMISE_STATE::RESOLVED;
            self->obj->state|= PROMISE_STATE::CLOSED  ;
        },[=]( V value ){
            self->obj->value = value; /*-------------*/
            self->obj->rej_clb.emit(value); /*-------*/
            self->obj->fin_clb.emit(/*-*/); /*-------*/
            self->obj->state = PROMISE_STATE::FINISHED;
            self->obj->state|= PROMISE_STATE::REJECTED;
            self->obj->state|= PROMISE_STATE::CLOSED  ;
        });

    }

public:

   ~promise_t() noexcept { if( obj.count()>1 ){ return; } emit(); }

    promise_t( const NODE_CLB& cb ) noexcept : obj( new NODE() ) {
        obj->node_clb=cb; obj->state=PROMISE_STATE::OPEN;
    }

    promise_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool is_finished() const noexcept { return obj->state & PROMISE_STATE::FINISHED; }

    bool is_resolved() const noexcept { return obj->state & PROMISE_STATE::RESOLVED; }

    bool is_rejected() const noexcept { return obj->state & PROMISE_STATE::REJECTED; }

    bool is_pending () const noexcept { return obj->state & PROMISE_STATE::PENDING ; }

    bool is_closed  () const noexcept { return obj->state & PROMISE_STATE::CLOSED  ; }

    bool has_value  () const noexcept { return obj->value.has_value(); }

    uchar get_state () const noexcept { return obj->state; }

    /*─······································································─*/

    expected_t<T,V> get_value() const {

        if( obj->state & PROMISE_STATE::RESOLVED )
          { return obj->value.template as<T>();  }
        if( obj->state & PROMISE_STATE::REJECTED )
          { return obj->value.template as<V>();  }
        
        if( obj->state & PROMISE_STATE::FINISHED )
          { throw except_t( "invalid value" ); }
      elif( obj->state & PROMISE_STATE::CLOSED )
          { throw except_t( "promise is closed" ); }
      elif( obj->state & PROMISE_STATE::PENDING )
          { throw except_t( "promise still pending" ); } 
      else{ throw except_t( "something went wrong"  ); }

    }

    void close() const noexcept { off(); }

    void off() const noexcept {
        obj->state = PROMISE_STATE::CLOSED;
        process::clear( obj->tsk );
        obj->node_clb.free();
    }

    /*─······································································─*/

    expected_t<T,V> await() const { do {

        if( obj->state== PROMISE_STATE::UNDEFINED ){ break; }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED    |
            /*--------*/ PROMISE_STATE::PENDING  )){ break; }
            
    invoke(); } while(0); 

        auto self = type::bind(this); process::await([=](){
            while( self->is_pending() ){ return  1; } 
            /*------------------------*/ return -1;
        }); return get_value(); 
    
    }

    /*─······································································─*/

    void emit() const noexcept {

        if( obj->state== PROMISE_STATE::UNDEFINED ){ return; }
        if( obj->state&( PROMISE_STATE::FINISHED  |
            /*--------*/ PROMISE_STATE::CLOSED    |
            /*--------*/ PROMISE_STATE::PENDING  )){ return; }

        auto self = type::bind( this );
        obj->tsk  = process::add([=](){ 
             self->invoke(); return -1; 
        });

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

    template< class T, class... V >
    promise_t<null_t,except_t> resolve( T cb, const V&... args ) {
    return promise_t<null_t,except_t>([=]( res_t<null_t> res, rej_t<except_t> rej ){

        process::add( coroutine::add( COROUTINE(){
        coBegin try {

            while( cb( args... )>=0 ){ return 1; }
            res( nullptr );

        } catch( except_t err ) {
            rej( err );
        } coFinish
        } ));

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace promise {

    template< class V >
    promise_t<V,except_t> all( const V& prom ) {
    return promise_t<V,except_t>([=]( res_t<V> res, rej_t<except_t>rej ){

        if( prom.empty() ){ rej( "iterator is empty" ); return; }
        ptr_t<ulong> idx ( 0UL, prom.size()-1 );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( *idx != 0 ){ auto x = prom[ *idx ].get_state();
            if( x & PROMISE_STATE::RESOLVED ){ *idx-=1;continue; }
            if( x & PROMISE_STATE::REJECTED )
              { rej( except_t( "there are rejected promises" ) ); coEnd; }
            *idx = *idx==1 ? prom.size()-1 : *idx-1 ; } 
            
            res( prom );

        coFinish
        }));

    }); }

    /*─······································································─*/

    template< class V >
    promise_t<V,except_t> any( const V& prom ) {
    return promise_t<V,except_t>([=]( res_t<V> res, rej_t<except_t>rej ){

        if( prom.empty() ){ rej( "iterator is empty" ); return; }
        ptr_t<ulong> idx ( 0UL, prom.size()-1 );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( *idx != 0 ){ auto x = prom[ *idx ].get_state();
            if( x & PROMISE_STATE::RESOLVED ){ res(prom); coEnd; }
            if( x & PROMISE_STATE::REJECTED ){ break; }
            *idx = *idx==1 ? prom.size()-1 : *idx-1 ; } 
            
            rej( except_t( "no fullfiled promises" ) );

        coFinish
        }));

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
