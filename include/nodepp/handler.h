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

    uint64_t msk1, msk2; queue_t<ptr_t<T>> que; 

    /*─······································································─*/

    uint64_t atob( void* address ) const noexcept {
    uint64_t raw = ( (uint64_t) address ) & (((uint64_t)-1)>>16);
    uint64_t msk = ( (uint64_t) msk1    ) & (((uint64_t)-1)>>16);
    uint64_t col =   (uint64_t) address   ^   (uint64_t) msk2 ;

        uint64_t sum = ( col^(col>>16)^(col>>32)^(col>>48)) & 0xffff;
        return ( raw ^ msk ) | ( sum << 48 );

    }

    void* btoa( uint64_t address ) const noexcept {
    uint64_t msk = ( (uint64_t)  msk1  )& (((uint64_t)-1)>>16);
    void*    raw = (void*)((address^msk)& (((uint64_t)-1)>>16) );
    uint64_t col = (uint64_t) raw       ^   (uint64_t) msk2 ;

        uint64_t sum = ( col^(col>>16)^(col>>32)^(col>>48)) & 0xffff;
        uint64_t out = ( address >>48) /*----------------*/ & 0xffff;
        return out==sum ? raw : nullptr ; 

    }

public: handler_t() : msk1( (uint64_t) this ), msk2( (uint64_t) &que ) {}

    uint64_t create() const noexcept { que.push( ptr_t<T>() ); return atob( que.last() ); }

    int remove( uint64_t address ) const noexcept {
        if  ( que.empty() )/**/{ return -1; }
        auto  ptr = que.as( btoa( address ) );
        if  ( ptr == nullptr ) { return -1; } 
    que.erase( ptr ); return 1; }

    bool has( uint64_t address ) const noexcept {
        if  ( que.empty() )/*-*/{ return false; }
        void* ptr = btoa( address );
        if  ( ptr == nullptr )  { return false; }
    return que.is_valid( ptr ); }

    int update( uint64_t address, T value ) const noexcept {
        if  ( que.empty() )/**/{ return -1; }
        auto  ptr = que.as( btoa( address ) );
        if  ( ptr == nullptr ) { return -1; }
        ptr->data = ptr_t<T>( 0UL, T(value) );
    return 1; }

    ptr_t<T> read( uint64_t address ) const noexcept {
        if  ( que.empty() )/*-*/{ return nullptr; }
        auto  ptr = que.as( btoa( address ) );
        if  ( ptr == nullptr )  { return nullptr; }
        auto  raw = ptr->data;
        if  ( raw.null() )/*--*/{ return nullptr; } 
    return raw; }

    /*─······································································─*/

    uint64_t get_handler( void* raw ) const noexcept {
        if( !que.is_valid( raw ) ){ return 0UL; }
    return atob( raw ); }

    /*─······································································─*/

    void  clear() const noexcept { /*--*/ que.clear(); }
    ulong  size() const noexcept { return que.size (); }
    bool  empty() const noexcept { return que.empty(); }

    /*─······································································─*/

    template< class F >
    void dispatch( uint64_t handle, const F& action ) const noexcept {
        auto state = this->read(handle); if( !state.null() ){
              action( *state );
        this->update( handle , *state ); }
    }

    /*─······································································─*/

    const queue_t<ptr_t<T>>* operator->() const noexcept { return &que; }
    const queue_t<ptr_t<T>>& get_data  () const noexcept { return  que; }
    const queue_t<ptr_t<T>>& get_queue () const noexcept { return  que; }

    /*─······································································─*/

    bool is_valid( uint64_t address ) const noexcept { return btoa( address )!=nullptr; }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/