/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_LISTENER
#define NODEPP_LISTENER

/*────────────────────────────────────────────────────────────────────────────*/

#include "map.h"
#include "event.h"
#include "optional.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, class... A > class listener_t {
protected:

    struct NODE { map_t<T,event_t<A...>> que; }; ptr_t<NODE> obj;

public:

    listener_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    template< class V >
    ptr_t<task_t> operator()( T val, V cb ) const noexcept { return on(val,cb); }

    /*─······································································─*/

    ptr_t<task_t> once( const T& val, function_t<void,A...> cb ) const noexcept {
        return obj->que[val].once( cb );
    }

    ptr_t<task_t> add( const T& val, function_t<int,A...> cb ) const noexcept {
        return obj->que[val].add( cb );
    }

    ptr_t<task_t> on( const T& val, function_t<void,A...> cb ) const noexcept {
        return obj->que[val].on( cb );
    }

    /*─······································································─*/

    bool   has( const T& val ) const noexcept { return obj->que.has( val ); }

    void clear( const T& val ) const noexcept { obj->que.erase( val ); }

    /*─······································································─*/

    ptr_t<T> keys() const noexcept { return obj->que.keys (); }
    bool    empty() const noexcept { return obj->que.empty(); }
    ulong    size() const noexcept { return obj->que.size (); }
    void    clear() const noexcept { /*--*/ obj->que.clear(); }
    void     free() const noexcept { /*--*/ obj->que.free (); }

    /*─······································································─*/

    optional_t<event_t<A...>> get( const T& val ) const noexcept {
        if( !has( val ) ){ return nullptr; }
        return obj->que[val];
    }

    void off( const T& val, ptr_t<task_t> address ) const noexcept {
         if( !has( val ) ){ return; } obj->que[val].off( address );
    }

    void emit( const T& val, const A&... args ) const noexcept {
         if( !has( val ) ){ return; } obj->que[val].emit( args... );
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
