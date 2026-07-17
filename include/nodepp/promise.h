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

#include "any.h"
#include "event.h"
#include "expected.h"
#include "initializer.h"

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

    using FINALLY = event_t< >; /*-------------------*/
    using RESOLVE = event_t<T>; /*-------------------*/
    using REJECT  = event_t<V>; /*-------------------*/
    using NODE_CLB= function_t<void,res_t<T>,rej_t<V>>;

protected:

    struct NODE {
        REJECT  rej_clb; /*----------*/
        RESOLVE res_clb; any_t value  ;
        FINALLY fin_clb; uchar state=0;
    };  ptr_t<NODE> obj;

public:

    promise_t( const NODE_CLB& cb ) noexcept: obj( new NODE() ) {
        obj->state|= PROMISE_STATE::PENDING ;
        auto self  = type::bind( this );
    process::add([=](){ cb([=]( T value ){
    if( self->obj->state & PROMISE_STATE::CLOSED ){ return; }
        self->obj->state = PROMISE_STATE::FINISHED;
        self->obj->state|= PROMISE_STATE::RESOLVED;
        self->obj->state|= PROMISE_STATE::CLOSED  ;
        self->obj->value = /**/ value ;
        self->obj->res_clb.emit(value);
        self->obj->fin_clb.emit(/*-*/);
    },[=]( V value ){
    if( self->obj->state & PROMISE_STATE::CLOSED ){ return; }
        self->obj->state = PROMISE_STATE::FINISHED;
        self->obj->state|= PROMISE_STATE::REJECTED;
        self->obj->state|= PROMISE_STATE::CLOSED  ;
        self->obj->value = /**/ value ;
        self->obj->rej_clb.emit(value);
        self->obj->fin_clb.emit(/*-*/);
    }); return -1; }); }

    promise_t() noexcept : obj( new NODE() ) {}

   ~promise_t() noexcept { if( obj.count()>1 ){ return; } emit(); }

    /*─······································································─*/

    bool  is_finished() const noexcept { return obj->state & PROMISE_STATE::FINISHED; }
    bool  is_resolved() const noexcept { return obj->state & PROMISE_STATE::RESOLVED; }
    bool  is_rejected() const noexcept { return obj->state & PROMISE_STATE::REJECTED; }
    bool  is_pending () const noexcept { return obj->state & PROMISE_STATE::PENDING ; }
    bool  is_closed  () const noexcept { return obj->state & PROMISE_STATE::CLOSED  ; }
    bool  has_value  () const noexcept { return obj->value.has_value(); }
    uchar get_state  () const noexcept { return obj->state; }

    /*─······································································─*/

    expected_t<T,V> get_value() const {

        if  ( obj->state & PROMISE_STATE::RESOLVED )
            { return obj->value.template as<T>();  }
        if  ( obj->state & PROMISE_STATE::REJECTED )
            { return obj->value.template as<V>();  }
            
        if  ( obj->state & PROMISE_STATE::FINISHED )
            { NODEPP_THROW_ERROR( "invalid value" ); }
        elif( obj->state & PROMISE_STATE::CLOSED )
            { NODEPP_THROW_ERROR( "promise is closed" ); }
        elif( obj->state & PROMISE_STATE::PENDING )
            { NODEPP_THROW_ERROR( "promise still pending" ); } 
        else{ NODEPP_THROW_ERROR( "something went wrong"  ); }

    }

    void   off() const noexcept { obj->state |= PROMISE_STATE::CLOSED; }
    void close() const noexcept { off(); }

    /*─······································································─*/

    int emit() const noexcept { do {

        if( obj->state==PROMISE_STATE::UNDEFINED ){ break; }
        if( !has_value() ) /*------------------*/ { break; }
 
        if( is_resolved() ){
            obj->res_clb.emit( obj->value.template as<T>() );
            obj->fin_clb.emit();
        return 1; }

        if( is_rejected() ){
            obj->rej_clb.emit( obj->value.template as<V>() );
            obj->fin_clb.emit();
        return 1; }

    } while(0); return -1; }

    /*─······································································─*/

    expected_t<T,V> await() const { do {

        if   ( obj->state==PROMISE_STATE::UNDEFINED ){ break; }
        while( is_pending() ){ process::next(); } 

    return get_value(); } while(0); return nullptr; }

    /*─······································································─*/

    template< class U, class P, class Q >
    promise_t& join( const U cb_then, const P cb_fail, const Q cb_final ) noexcept {
    return then( cb_then ).fail( cb_fail ).finally( cb_final ); }

    template< class U, class P >
    promise_t& join( const U cb_then, const P cb_fail ) noexcept {
    return then( cb_then ).fail( cb_fail ); }

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
        ptr_t<ulong> idx ( 2UL, 0x00 );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( idx[0]!=prom.size() ){ coNext; do {
            auto x = prom[idx[1]]; idx[1]=idx[1] ++ % prom.size(); 
            if   ( x.is_resolved() ){ idx[0] ++; continue; }
            if   ( x.is_rejected() ){ coGoto(2); } *idx=0; } while(0); }

            
            coYield(1); res( prom ); /*--------------------------------*/ coEnd;
            coYield(2); rej( except_t( "there are rejected promises" ) ); coEnd;

        coFinish
        }));

    }); }

    /*─······································································─*/

    template< class T, class... V >
    promise_t<null_t,except_t> resolve( T cb, const V&... args ) {
    return promise_t<null_t,except_t>([=]( 
           res_t<null_t> res, rej_t<except_t> rej 
    ){  function_t<int,V...> clb ( cb );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            coWait( clb( args... )>=0 );
            res   ( nullptr );

        coFinish } ));

    }); }

    /*─······································································─*/

    template< class V >
    promise_t<V,except_t> any( initializer_t<V> prom ) {
    return promise_t<V,except_t>([=]( res_t<V> res, rej_t<except_t>rej ){

        if( prom.empty() ){ rej( "iterator is empty" ); return; }
        ptr_t<ulong> idx ( 0UL, 0x00 );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            do{ coNext; do { auto x=prom[idx[0]];
            if( x.is_resolved() ){ coGoto(1); }
            if( x.is_rejected() ){ coGoto(2); } 
            idx[0] = idx[0] ++ % prom.size(); } while(0); } while(1);
            
            coYield(1); res( prom[idx[0]] ); /*------------------*/ coEnd;
            coYield(2); rej( except_t( "no fullfiled promises" ) ); coEnd;

        coFinish
        }));

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/
