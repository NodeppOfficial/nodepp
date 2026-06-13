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

namespace nodepp { template< class U, class V, ulong SIZE=NODEPP_HASH_TABLE_SIZE > class map_t {
protected:

    using T = pair_t < U, V >; 

    /*─······································································─*/

    queue_t<queue_t<void*>> table;
    queue_t<T> /*--------*/ queue;

    /*─······································································─*/

    struct NODE {
    
        NODE* left = nullptr;
        NODE* right= nullptr;
        void* value= nullptr;
    
       ~NODE(){
        if( left  ){ delete left  ; left  =nullptr; }
        if( right ){ delete right ; right =nullptr; }}

    };  ptr_t<NODE> obj;

    /*─······································································─*/

    void*& get_address( int key ) const noexcept {
        
        NODE*  tmp = &obj;
        while( key != 0 ){ switch( key & 0x1 ){

        case 0: do { if( tmp->left==nullptr ){ 
            tmp->left /*--*/ = new NODE;
        }   tmp = tmp->left; } while(0); break;

        case 1: do { if( tmp->right==nullptr ){ 
            tmp->right /*--*/ = new NODE;
        }   tmp = tmp->right; } while(0); break;

        } key >>= 1; } 

        if( tmp->value == nullptr ){
            table.push( queue_t<void*>() );
            tmp->value = table.last();
        }

        return tmp->value;

    }

    /*─······································································─*/

    void append( const T& pair ) const noexcept {

        auto  key = string::to_string ( pair.first );
        ulong idx = encoder::hash::get( key , SIZE );

        auto  que = table.as( get_address(idx) )->data;
        auto  n   = que  .first();

        while( n!=nullptr ){ auto itm = queue.as( n->data );
        if   ( itm == nullptr ){ break; }
        if   ( itm->data.first == pair.first ){
        /*--*/ itm->data.second = pair.second;
        break; } n = n->next; }

        queue.push( pair ); que.push( queue.last() );

    }

public:

    template< ulong N >
    map_t( const T (&args) [N] ) noexcept : obj( 0UL ) {
    for  ( auto &x: args ) { append(x); }}

    map_t( null_t ) noexcept : obj( 0UL ){}

    map_t() /*---*/ noexcept : obj( 0UL ){}

    /*─······································································─*/
    
    explicit operator bool(void) const noexcept { return !empty(); }

    V& operator[]( const U& id ) const noexcept {

        auto  key = string::to_string ( id );
        ulong idx = encoder::hash::get( key, SIZE );

        auto  que = table.as( get_address(idx) )->data;
        auto  n   = que  .first();

        while( n!=nullptr ){ auto itm = queue.as( n->data );
        if   ( itm == nullptr ){ break; }
        if   ( itm->data.first == id )
        /**/ { return itm->data.second; }
        n = n->next; } 

        append({ id, V() });
        return queue.last()->data.second;

    }

    /*─······································································─*/

    bool /*------*/ empty() const noexcept { return queue.empty(); }
    ulong /*------*/ size() const noexcept { return queue.size (); }
    ptr_t<T> /*---*/ data() const noexcept { return queue.data (); }
    ptr_t<T> /*----*/ get() const noexcept { return queue.data (); }
    const queue_t<T>& raw() const noexcept { return queue; }

    /*─······································································─*/

    bool has( const U& id ) const noexcept {

        auto  key = string::to_string ( id );
        ulong idx = encoder::hash::get( key, SIZE );

        auto  que = table.as( get_address(idx) )->data;
        auto  n   = que  .first();

        while( n!=nullptr ){ auto itm = queue.as( n->data );
        if   ( itm == nullptr ){ break; }
        if   ( itm->data.first == id )
        /**/ { return true; }
        n = n->next; } 
        
        return false;

    }

    /*─······································································─*/

    void map( function_t<void,T&> callback ) const noexcept {
         queue.map( callback );
    }

    /*─······································································─*/

    array_t<U> keys() const noexcept { queue_t<U> result;
        auto x = queue.first(); while( x!=nullptr ){
            result.push( x->data.first ); x=x->next;
        }   return result.data();
    }

    /*─······································································─*/

    void erase( const U& id ) const noexcept {

        auto  key = string::to_string ( id );
        ulong idx = encoder::hash::get( key, SIZE );

        auto  que = table.as( get_address(idx) )->data;
        auto  n   = que  .first();

        while( n != nullptr ){ 
        auto itm = queue.as ( n->data );
        if   ( itm == nullptr ){ return; }
        if   ( itm->data.first==id )
             { queue.erase(itm); break ; } n = n->next; } 
        
        if( n==nullptr ){ return; } que.erase(n);

    }

    void erase( void* node ) const noexcept {
        auto x = queue.as( node );
        if ( x== nullptr ){ return; }
        erase( x->data.first );
    }

    void erase() const noexcept {
        auto x = table.first(); while( x!=nullptr ){
            x->data.erase();
        x = x->next; } queue.erase();
    }

    void clear() const noexcept { erase(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/