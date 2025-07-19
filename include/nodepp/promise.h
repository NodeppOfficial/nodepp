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

#include "generator.h"
#include "expected.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace promise {

    template< class T, class V > void* resolve( 
        function_t<void,function_t<void,T>,function_t<void,V>> func,
        function_t<void,T> res, function_t<void,V> rej
    ){  
        ptr_t<bool> state = new bool(1); generator::promise::resolve task;
        return process::add( task, state, func, [=]( T data ){
           if( *state!=1 ){ return; } res(data); *state=0;
        }, [=]( V data ) {
           if( *state!=1 ){ return; } rej(data); *state=0;
        } ); return (void*) &state;
    }

    template< class T > void* resolve( 
        function_t<void,function_t<void,T>> func,
        function_t<void,T> res
    ){  
        ptr_t<bool> state = new bool(1); generator::promise::resolve task;
        return process::add( task, state, func, [=]( T data ){
           if( *state!=1 ){ return; } res(data); *state=0;
        } ); return (void*) &state;
    }

    /*─······································································─*/

    template< class T, class V > expected_t<T,V> await( 
        function_t<void,function_t<void,T>,function_t<void,V>> func 
    ){   
        ptr_t<bool> state = new bool(1); T res; V rej; bool x=0;
        generator::promise::resolve task;
        process::await( task, state, func, [&]( T data ){
            if( *state!=1 ){ return; } res = data; *state=0; x=1;
        }, [&]( V data ){
            if( *state!=1 ){ return; } rej = data; *state=0; x=0;
        }); if( x ){ return res; } return rej;
    }

    template< class T > T await( 
        function_t<void,function_t<void,T>> func 
    ){  
        ptr_t<bool> state = new bool(1); T out; 
        generator::promise::resolve task;
        process::await( task, state, func, [&]( T data ){
            if( *state!=1 ){ return; } out = data; *state=0;
        }); return out;
    }
    
    /*─······································································─*/

    void clear( void* address ){ process::clear( address ); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, class V > class promise_t { 
protected:

    struct NODE {
        function_t<void,function_t<void,T>,function_t<void,V>> main_func;
        void* addr = nullptr; uchar state=0;
    };  ptr_t<NODE> obj;

    event_t<T> onDone; 
    event_t<V> onFail;
    event_t<>  onFinally;

public:

    template< class U >
    promise_t& then( const U cb )    noexcept { obj->state=1; onDone.once(cb);    return (*this); }
    
    template< class U >
    promise_t& fail( const U cb )    noexcept { obj->state=1; onFail.once(cb);    return (*this); }
    
    template< class U >
    promise_t& finally( const U cb ) noexcept { /*---------*/ onFinally.once(cb); return (*this); }

    /*─······································································─*/

    void resolve() const noexcept { if( obj->state!=1 ){ return; }
        obj->state= 0; auto self = type::bind(this);
        obj->addr = promise::resolve<T,V>( obj->main_func, 
            [=]( T res ){ self->onDone.emit(res); self->onFinally.emit(); self->free(); },
            [=]( V rej ){ self->onFail.emit(rej); self->onFinally.emit(); self->free(); }
        ); 
    }

    void clear() const noexcept { process::clear( obj->addr ); }
    void  free() const noexcept { onDone.clear(); onFail.clear(); onFinally.clear(); }
    expected_t<T,V> await() const noexcept { return promise::await<T,V>( obj->main_func ); }

    /*─······································································─*/

    promise_t() noexcept : obj( new NODE ) {}
   ~promise_t() noexcept { if( obj.count()>1 ){ return; } resolve(); }
    promise_t( const decltype(NODE::main_func)& cb ) noexcept : obj( new NODE ) { obj->main_func=cb; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
