/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WAIT
#define NODEPP_WAIT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class wait_t { 
protected:

    using NODE = function_t<bool,T>; ptr_t<queue_t<NODE>> obj;

public: wait_t() noexcept : obj( new queue_t<NODE>() ) {}
    
    /*─······································································─*/

    void* operator()( T val, function_t<void> func ) const noexcept { return on(val,func); }
    
    /*─······································································─*/

    void off( void* address ) const noexcept { process::clear( address ); }

    void* once( T val, function_t<void> func ) const noexcept {
        if( obj->size() >= MAX_EVENTS ) { return nullptr; }
        ptr_t<bool> out = new bool(1); obj->push([=]( T arg ){
            if( *out != 0 && val == arg  ){ func(); }
            *out = 0; return *out;
        }); return &out;
    }

    void* on( T val, function_t<void> func ) const noexcept {
        if( obj->size() >= MAX_EVENTS ) { return nullptr; }
        ptr_t<bool> out = new bool(1); obj->push([=]( T arg ){
            if( *out != 0 && val == arg  ){ func(); } 
            return *out;
        }); return &out;
    }
    
    /*─······································································─*/

    bool  empty() const noexcept { return obj->empty(); }
    ulong  size() const noexcept { return obj->size(); }
    void  clear() const noexcept { obj->clear(); }
    
    /*─······································································─*/

    void emit( const T& arg ) const noexcept {
        auto x = obj->first(); while( x != nullptr ){
        auto y = x->next; 
            if( !x->data( arg ) ){ obj->erase(x); }
        x = y; }
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif