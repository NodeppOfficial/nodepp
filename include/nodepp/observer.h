/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_OBSERVER
#define NODEPP_OBSERVER

/*────────────────────────────────────────────────────────────────────────────*/

#include "wait.h" 
#include "type.h"
#include "map.h"
#include "any.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class observer_t {
private:

    map_t <string_t,any_t> /*-*/ list ;
    wait_t<string_t,any_t,any_t> event;
    
    using P=type::pair<string_t,any_t>;
    using G=function_t<int,any_t,any_t>;
    using F=function_t<void,any_t,any_t>;

public: observer_t() noexcept {}
    
    /*─······································································─*/

    template< ulong N >
    observer_t( const P (&args) [N] ) noexcept { ulong x=N; while( x-->0 ){
        list[args[x].first] = args[x].second;
    }}

    /*─······································································─*/

    template< class F >
    void set( string_t name, const F& value ) const {
        if( !list.has( name ) ){ throw except_t("field not found:",name); }
        auto n = list[ name ]; event.emit( name, n, value );
        /*----*/ list[ name ]= value;        
    }

    const any_t get( string_t name ) const { if( !list.has( name ) ){
        throw except_t( "field not found:", name ); 
    }   return list[ name ]; }
    
    /*─······································································─*/

    const any_t operator[]( string_t name ) const { return get( name ); }
    
    /*─······································································─*/

    void off( ptr_t<task_t> addr ) const noexcept { event.off(addr); }

    ptr_t<task_t> once( string_t name, F func ) const noexcept {
        if( !list.has( name ) ){ return nullptr; }
        if( func.empty() )/*-*/{ return nullptr; }
        return event.once( name, func );
    }

    ptr_t<task_t> add( string_t name, G func ) const noexcept {
        if( !list.has( name ) ){ return nullptr; }
        if( func.empty() )/*-*/{ return nullptr; }
        return event.add( name, func );
    }

    ptr_t<task_t> on( string_t name, F func ) const noexcept {
        if( !list.has( name ) ){ return nullptr; }
        if( func.empty() )/*-*/{ return nullptr; }
        return event.on( name, func );
    }
    
    /*─······································································─*/

    void clear() const noexcept { list.clear(); event.clear(); }

    bool empty() const noexcept { return list.empty(); }

    ulong size() const noexcept { return list.size(); }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/
