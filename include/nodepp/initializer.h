/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_INITIALIZER
#define NODEPP_INITIALIZER

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class initializer_t : public ptr_t<T> { 
    public: initializer_t() noexcept : ptr_t<T>(){}

    /*─······································································─*/
    
    initializer_t( const ptr_t<T>& arr ) noexcept : ptr_t<T>( arr ) {}

    template< class... V >
    initializer_t( const T& head, const V&... tail ) noexcept {
        if( this->empty() ) this->resize( sizeof...(V) + 1 );
        ulong index = 0; iterator::map([&]( const T& item ){
           this->value_[index] = (T)item; ++index;
        }, head, tail... );
    }
    
    initializer_t( const queue_t<T>& arr ) noexcept {
        this->resize( arr.size() ); auto raw = arr.data();
        type::copy( raw.begin(), raw.end(), this->begin() );
    }
    
    initializer_t( const array_t<T>& arr ) noexcept {
        this->resize( arr.size() ); 
        type::copy( arr.begin(), arr.end(), this->begin() );
    }

    template< ulong N >
    initializer_t( const T (&arr) [N] ) noexcept {
        this->reset(); this->resize( N );
        type::copy( arr, arr+N, this->begin() );
    }

    virtual ~initializer_t() noexcept {}

    /*─······································································─*/

    int index_of( function_t<bool,T> func ) const noexcept {
        int out=0; auto addr = this->begin(); while( addr != this->end() ){
            if( func( *addr ) ){ return out; }
        ++addr; ++out; } return -1;
    }

    int count( function_t<bool,T> func ) const noexcept {
        int out=0; auto addr = this->begin(); while( addr != this->end() ){
            if( func( *addr ) ){ ++out; }
        ++addr; } return -1;
    }

    /*─······································································─*/

    bool some( function_t<bool,T> func ) const noexcept {
        auto addr = this->begin(); while( addr != this->end() ){
            if( func(*addr)==1 ){ return 1; }
        ++addr; } return 0;
    }

    bool none( function_t<bool,T> func ) const noexcept {
        auto addr = this->begin(); while( addr != this->end() ){
            if( func(*addr)==1 ){ return 0; }
        ++addr; } return 1;
    }

    bool every( function_t<bool,T> func ) const noexcept {
        auto addr = this->begin(); while( addr != this->end() ){
            if( func(*addr)==0 ){ return 0; }
        ++addr; } return 1;
    }

    void map( function_t<void,T&> func ) const noexcept {
        auto addr = this->begin(); while( addr != this->end() ){
        func(*addr); ++addr; }
    }

    T reduce( function_t<T,T,T> func ) const noexcept {
        auto out = *this->begin(); auto addr = this->begin();
        while( addr != this->end() ){ ++addr;
               out = func( out, *addr );
        } return out;
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif