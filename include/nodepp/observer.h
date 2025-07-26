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

#include "event.h" 
#include "type.h"
#include "any.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class observer_t {
private:

    template< class A, class B, class C >
    struct NODE_ { A first; B second; C third; };

    using V = any_t;
    using U = string_t; 
    using E = event_t<V,V>;
    using T = NODE_<U,V,E>;
    using P = type::pair<U,V>;

public: observer_t() noexcept {} queue_t<T> node;
    
    /*─······································································─*/

    template< ulong N >
    observer_t ( const P (&args) [N] ) noexcept {
        node.clear(); ulong x=N; while( x-->0 ){
            T item; // memset( &item, 0, sizeof(T) );
            item.second = args[x].second;
            item.first  = args[x].first;
            node.push(item);
        }
    }

    virtual ~observer_t() noexcept {}
    
    /*─······································································─*/

    void off( void* address ) const noexcept { 
        if( !address ){ return; } *((int*)address) = -1; 
    }

    template< class F >
    void* once( const U& name, F func ) const noexcept {
        auto n = node.first(); while( n!=nullptr ){
             console::log( n->data.first, name );
        if ( n->data.first == name ){
             return n->data.third.once( func );
        }    n = n->next; } return nullptr;
    }

    template< class F >
    void* on( const U& name, F func ) const noexcept {
        auto n = node.first(); while( n!=nullptr ){
        if ( n->data.first == name ){
             return n->data.third.on( func );
        }    n = n->next; } return nullptr;
    }
    
    /*─······································································─*/
    
    void set( function_t<observer_t,observer_t> func ) const {
        observer_t obj = func( *this ); 
        auto   n = obj.node.first();
        while( n!=nullptr ){ 
            this->set( n->data.first, n->data.second );
            n = n->next;
        }
    }
    
    /*─······································································─*/

    template< class F >
    void set( const U& name, const F& value ) const {
        auto n = node.first(); while( n!=nullptr ){
        if ( n->data.first == name ){
             n->data.third.emit( n->data.second, value );
             n->data.second = value; return;
        }    n = n->next; }   
             throw except_t("field not found:",name);
    }
    
    /*─······································································─*/
    
    template< class V, ulong N >
    void set( const V (&args) [N] ) const { for( ulong x=0; x<N; ++x ){
        this->set( args[x].first, args[x].second );
    }}

    /*─······································································─*/

    const V get( const U& name ) const {
        auto n = node.first(); while( n!=nullptr ){
        if ( n->data.first == name ){
             return n->data.second;
        }    n = n->next; }   
             throw except_t( "field not found:", name ); 
             return (const V)(0);
    }
    
    /*─······································································─*/

    bool empty() const noexcept { return node.empty(); }

    ulong size() const noexcept { return node.size(); }

    /*─······································································─*/

    void clear( string_t name ) const noexcept { 
        auto n = node.first(); while( n!=nullptr ){
        if ( n->data.first == name ){
             n->data.third.clear();
        }    n = n->next; }
    }

    void clear() const noexcept { 
        auto n = node.first(); while( n!=nullptr ){
             n->data.third.clear();
             n = n->next;
        }
    }
    
    /*─······································································─*/

    const V operator[]( const U& name ) const {
        return get( name );
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
