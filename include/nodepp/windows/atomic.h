/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_ATOMIC
#define NODEPP_WINDOWS_ATOMIC

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>
#include <winnt.h>
#include <intrin.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace atomic { 

    template< class T > struct is_atomic           : type::false_type{};

    template<>          struct is_atomic<int>      : type::true_type {};
    template<>          struct is_atomic<uint>     : type::true_type {};
    template<>          struct is_atomic<bool>     : type::true_type {};
    template<>          struct is_atomic<char>     : type::true_type {};
    template<>          struct is_atomic<long>     : type::true_type {};
    template<>          struct is_atomic<short>    : type::true_type {};
    template<>          struct is_atomic<uchar>    : type::true_type {};
    template<>          struct is_atomic<llong>    : type::true_type {};
    template<>          struct is_atomic<ulong>    : type::true_type {};
    template<>          struct is_atomic<ushort>   : type::true_type {};
    template<>          struct is_atomic<ullong>   : type::true_type {};

//  template<>          struct is_atomic<float>    : type::true_type {};
//  template<>          struct is_atomic<double>   : type::true_type {};

    template<>          struct is_atomic<wchar_t>  : type::true_type {};
    template<>          struct is_atomic<char16_t> : type::true_type {};
    template<>          struct is_atomic<char32_t> : type::true_type {};

    template< class T > struct is_atomic<T*>       : type::true_type{};

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { 
template< class T, class = typename type::enable_if<atomic::is_atomic<T>::value,T>::type >
class atomic_t   { private: volatile T value; protected: 

    void cpy( const atomic_t& other ) noexcept { 
         memcpy( &value, &other.value, sizeof( T ) );
    }

    void mve( atomic_t&& other )      noexcept { 
         memmove( &value, &other.value, sizeof( T ) );
    }

public:

    atomic_t( atomic_t&& other ) noexcept { mve(type::move(other)); }

    atomic_t( const atomic_t& other ) noexcept { cpy(other); }

    atomic_t( T _val_ ) noexcept : value( _val_ ) {}

    atomic_t() noexcept : value( T{} ) {}

public:

    bool compare( T& expected, T desired ) const noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return type::cast<T>( ::InterlockedCompareExchange  ( (LONG   volatile *)&value, (LONG)  desired, (LONG)  expected ) ); break;
        case 8 : return type::cast<T>( ::InterlockedCompareExchange64( (LONG64 volatile *)&value, (LONG64)desired, (LONG64)expected ) ); break;
    }}

    T swap( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return type::cast<T>( ::InterlockedExchange  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return type::cast<T>( ::InterlockedExchange64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    /*─······································································─*/

    void set( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: type::cast<T>( ::InterlockedExchange  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : type::cast<T>( ::InterlockedExchange64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    T get() const noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return type::cast<T>( ::InterlockedExchangeAdd  ( (LONG   volatile *)&value, 0 ) ); break;
        case 8 : return type::cast<T>( ::InterlockedExchangeAdd64( (LONG64 volatile *)&value, 0 ) ); break;
    }}

    /*─······································································─*/

    T _or( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return type::cast<T>( ::InterlockedOr  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return type::cast<T>( ::InterlockedOr64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    T _and( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return type::cast<T>( ::InterlockedAnd  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return type::cast<T>( ::InterlockedAnd64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    T _xor( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return type::cast<T>( ::InterlockedXor  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return type::cast<T>( ::InterlockedXor64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    /*─······································································─*/

    T add( T new_val ) noexcept { 
        size_t scale = 1; if ( type::is_pointer<T>::value ) 
             { scale = sizeof( typename type::remove_pointer<T>::type ); }
    switch( sizeof( T ) ){
        case 2 : case 4 : 
        default: return type::cast<T>( ::InterlockedExchangeAdd  ( (LONG   volatile *)&value, (LONG)  new_val * scale ) ); break;
        case 8 : return type::cast<T>( ::InterlockedExchangeAdd64( (LONG64 volatile *)&value, (LONG64)new_val * scale ) ); break;
    }}

    T sub( T new_val ) noexcept { 
        size_t scale = 1; if ( type::is_pointer<T>::value ) 
             { scale = sizeof( typename type::remove_pointer<T>::type ); }
    switch( sizeof( T ) ){
        case 2 : case 4 : 
        default: return type::cast<T>( ::InterlockedExchangeAdd  ( (LONG   volatile *)&value, -(LONG)  new_val * scale ) ); break;
        case 8 : return type::cast<T>( ::InterlockedExchangeAdd64( (LONG64 volatile *)&value, -(LONG64)new_val * scale ) ); break;
    }}

public:

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, atomic_t& >::type
    operator&=( T value ) noexcept { _and(value); return *this; }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, atomic_t& >::type
    operator|=( T value ) noexcept { _or (value); return *this; }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, atomic_t& >::type
    operator^=( T value ) noexcept { _xor(value); return *this; }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, atomic_t& >::type
    operator-=( T value ) noexcept {  sub(value); return *this; }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, atomic_t& >::type
    operator+=( T value ) noexcept {  add(value); return *this; }

    /*─······································································─*/

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, T >::type
    operator--() /*-------------*/ noexcept { return sub(1) - 1; }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, T >::type
    operator++() /*-------------*/ noexcept { return add(1) + 1; }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, T >::type
    operator--(int) /*----------*/ noexcept { return sub(1); }

    template< typename U = T >
    typename type::enable_if< !type::is_pointer<U>::value, T >::type
    operator++(int) /*----------*/ noexcept { return add(1); }

    /*─······································································─*/

    atomic_t& operator =( T value ) noexcept { set(value); return *this; }

    /*─······································································─*/

    bool operator==( T value ) const noexcept { return get() == value; }
    bool operator>=( T value ) const noexcept { return get() >= value; }
    bool operator<=( T value ) const noexcept { return get() <= value; }
    bool operator> ( T value ) const noexcept { return get() >  value; }
    bool operator< ( T value ) const noexcept { return get() <  value; }
    bool operator!=( T value ) const noexcept { return get() != value; }

    explicit operator T() /**/ const noexcept { return get(); }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/