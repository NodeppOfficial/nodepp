/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_ATOMIC
#define NODEPP_POSIX_ATOMIC

/*────────────────────────────────────────────────────────────────────────────*/

#include <stdbool.h>

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
class atomic_t   { private: T value; protected: 

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

    T get() const noexcept { 
        return __atomic_load_n( &value, __ATOMIC_ACQUIRE );
    }

    void set( T new_val ) noexcept {
        __atomic_store_n( &value, new_val, __ATOMIC_RELEASE );
    }

    /*─······································································─*/

    T _and( T new_val ) noexcept {
        return __atomic_fetch_and( &value, new_val, __ATOMIC_SEQ_CST );
    }

    T _xor( T new_val ) noexcept {
        return __atomic_fetch_xor( &value, new_val, __ATOMIC_SEQ_CST );
    }

    T _or( T new_val ) noexcept {
        return __atomic_fetch_or( &value, new_val, __ATOMIC_SEQ_CST );
    }

    /*─······································································─*/

    T add( T new_val ) noexcept {
        size_t scale = 1; if ( type::is_pointer<T>::value ) 
             { scale = sizeof( typename type::remove_pointer<T>::type ); }
        return __atomic_fetch_add( &value, new_val * scale, __ATOMIC_SEQ_CST );
    }

    T sub( T new_val ) noexcept {
        size_t scale = 1; if ( type::is_pointer<T>::value ) 
             { scale = sizeof( typename type::remove_pointer<T>::type ); }
        return __atomic_fetch_sub( &value, new_val * scale, __ATOMIC_SEQ_CST );
    }

    /*─······································································─*/

    T swap( T new_val ) noexcept {
        return __atomic_exchange_n( &value, new_val, __ATOMIC_SEQ_CST );
    }

    bool compare( T& expected, T desired ) const noexcept {
        return __atomic_compare_exchange_n( &value, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
    }

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