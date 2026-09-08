/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MEMORY_DMA
#define NODEPP_MEMORY_DMA

/*────────────────────────────────────────────────────────────────────────────*/

#include "any.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace  nodepp { template< class T > class handler_t {
protected:

    /*─······································································─*/

    uchar_64 mask; queue_t<ptr_t<T>> que; 

    /*─······································································─*/

    uchar_64 atob( void*    address ) const noexcept { return encoder::ofuscator::atob( address, mask ); }
    void*    btoa( uchar_64 address ) const noexcept { return encoder::ofuscator::btoa( address, mask ); }

public: handler_t() : mask( (uchar_64) &que ) {}

    uchar_64 create() const noexcept { que.push( ptr_t<T>() ); return atob( que.last() ); }

    int remove( uchar_64 address ) const noexcept {
        if  ( que.empty() )/**/{ return -1; }
        auto  ptr = que.as( btoa( address ) );
        if  ( ptr == nullptr ) { return -1; } 
    que.erase( ptr ); return 1; }

    bool has( uchar_64 address ) const noexcept {
        if  ( que.empty() )/*-*/{ return false; }
        void* ptr = btoa( address );
        if  ( ptr == nullptr )  { return false; }
    return que.is_valid( ptr ); }

    int update( uchar_64 address, T value ) const noexcept {
        if  ( que.empty() )/**/{ return -1; }
        auto  ptr = que.as( btoa( address ) );
        if  ( ptr == nullptr ) { return -1; }
        ptr->data = ptr_t<T>( 0UL, T(value) );
    return 1; }

    ptr_t<T> read( uchar_64 address ) const noexcept {
        if  ( que.empty() )/*-*/{ return nullptr; }
        auto  ptr = que.as( btoa( address ) );
        if  ( ptr == nullptr )  { return nullptr; }
        auto  raw = ptr->data;
        if  ( raw.null() )/*--*/{ return nullptr; } 
    return raw; }

    /*─······································································─*/

    uchar_64 get_handler( void* raw ) const noexcept {
        if( !que.is_valid( raw ) ){ return 0UL; }
    return atob( raw ); }

    /*─······································································─*/

    void  clear() const noexcept { /*--*/ que.clear(); }
    ulong  size() const noexcept { return que.size (); }
    bool  empty() const noexcept { return que.empty(); }

    /*─······································································─*/

    template< class F >
    void dispatch( uchar_64 handle, const F& action ) const noexcept {
        auto state = this->read(handle); if( !state.null() ){
              action( *state );
        this->update( handle , *state ); }
    }

    /*─······································································─*/

    const queue_t<ptr_t<T>>* operator->() const noexcept { return &que; }
    const queue_t<ptr_t<T>>& get_data  () const noexcept { return  que; }
    const queue_t<ptr_t<T>>& get_queue () const noexcept { return  que; }

    /*─······································································─*/

    bool is_valid( uchar_64 address ) const noexcept { return btoa( address )!=nullptr; }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/