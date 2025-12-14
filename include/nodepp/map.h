/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MAP
#define NODEPP_MAP

/*────────────────────────────────────────────────────────────────────────────*/

#include "encoder.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template<class U, class V> class map_t {
protected:

    using T    = type::pair< U, V >;
    using LIST = queue_t<void*>;

    struct NODE {
        ptr_t<LIST> table;
        queue_t<T>  queue;
    };  ptr_t<NODE> obj;

protected:

    void append( const T& pair ) const noexcept {

        auto  key = string::to_string(pair.first);
        ulong idx = encoder::hash::get(key);
        auto  n   = obj->table[idx].first();

        while( n!=nullptr ){
        auto itm = obj->queue.as(n->data); if( itm==nullptr ){ break; }
        if ( itm->data.first == pair.first ){
             itm->data.second = pair.second;
        return; } n = n->next; }

        obj->queue.push( pair );
        obj->table[idx].push( obj->queue.last() );

    }

public:

    template< ulong N >
    map_t( const T (&args) [N] ) noexcept : obj(new NODE()) {
        obj->table = ptr_t<LIST>( HASH_TABLE_SIZE );
        for( auto &x: args ) { append(x); }
    }

    map_t() noexcept : obj(new NODE()) {
        obj->table = ptr_t<LIST>( HASH_TABLE_SIZE );
    }

    virtual ~map_t() noexcept {}

    /*─······································································─*/

    V& operator[]( const U& id ) const noexcept {

        auto key = string::to_string( id );
        ulong idx= encoder::hash::get(key);
        auto n   = obj->table[idx].first();

        while( n!=nullptr ){
        auto itm = obj->queue.as(n->data); if( itm==nullptr ){ break; }
        if ( itm->data.first==id ){ return itm->data.second; }
        n = n->next; } append({ id, V() });

        return obj->queue.last()->data.second;

    }

    /*─······································································─*/

    bool      empty() const noexcept { return obj->queue.empty(); }
    ulong      size() const noexcept { return obj->queue.size(); }
    ptr_t<T>   data() const noexcept { return obj->queue.data(); }
    ptr_t<T>    get() const noexcept { return obj->queue.data(); }
    queue_t<T>& raw() const noexcept { return obj->queue; }

    /*─······································································─*/

    bool has( const U& id ) const noexcept {

        auto  key = string::to_string( id );
        ulong idx = encoder::hash::get(key);
        auto  n   = obj->table[idx].first();

        while( n!=nullptr ){
        auto itm = obj->queue.as(n->data); if( itm==nullptr ){ break; }
        if ( itm->data.first==id ){ return true; } n = n->next; }

        return false;

    }

    /*─······································································─*/

    void map( function_t<void,T&> callback ) const noexcept {
         obj->queue.map( callback );
    }

    /*─······································································─*/

    array_t<U> keys() const noexcept { queue_t<U> result;
        auto x = obj->queue.first(); while( x!=nullptr ){
            result.push( x->data.first ); x=x->next;
        }   return result.data();
    }

    /*─······································································─*/

    void erase( const U& id ) const noexcept {

        auto  key = string::to_string( id );
        ulong idx = encoder::hash::get(key);
        auto  n   = obj->table[idx].first();

        while( n!=nullptr ){
        auto itm = obj->queue.as(n->data); if( itm==nullptr ){ return; }
        if ( itm->data.first==id ){ obj->queue.erase(itm); break; }
        n = n->next; } if( n==nullptr ) { return; }

        obj->table[idx].erase( n );

    }

    void erase() const noexcept {
        for( auto x: obj->table ){ x.erase(); }
        obj->queue.erase();
    }

    void clear() const noexcept { erase(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif