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

namespace nodepp { 
template< class T, class = typename type::enable_if<type::is_trivially_copyable<T>::value,T>::type >
class atomic_t   { private: T value; protected: 

    void cpy( const atomic_t& other ) noexcept { memcpy ( &value, &other.value, sizeof( T ) ); }

    void mve( atomic_t&& other ) /**/ noexcept { memmove( &value, &other.value, sizeof( T ) ); }

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

namespace nodepp { namespace atomic {

    inline void acquire(){ __atomic_thread_fence( __ATOMIC_ACQUIRE ); }
    inline void release(){ __atomic_thread_fence( __ATOMIC_RELEASE ); }
    
    template< class T, class... V >
    void fence( const T& callback, V... args ){
        acquire(); callback( args... );
        release();
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class atomic_ptr_t {
private:

    struct NODE {
        T* value = nullptr; atomic_t<ulong> count;
        /*---------------*/ ulong size=0;
    };  NODE* obj= nullptr;

    void release() noexcept {
        if( obj == nullptr ) /**/ { return ; }
        if( obj->count.sub(1)==1 ){ 
        if( obj->value ){ delete obj->value; }
            delete obj; 
        }   obj = nullptr;
    }

    void cpy( const atomic_ptr_t& other ) noexcept { release();
        if( other.null() )/**/{ return; }
        obj=other.obj; obj->count.add(1);
    }

    void mve( atomic_ptr_t&& other ) noexcept { release();
        if( other.null() )/**/{ return; }
        obj=other.obj; other.obj=nullptr;
    }

public: 

    atomic_ptr_t( T* value ) : obj( new NODE() ){
        obj->value = value ; obj->size=0;
        obj->count.set(1UL);
    }
    
    atomic_ptr_t() noexcept {}

   ~atomic_ptr_t() noexcept { release(); }

    /*─······································································─*/

    atomic_ptr_t( atomic_ptr_t&& other ) noexcept : obj(nullptr) { mve( type::move( other ) ); }
    atomic_ptr_t& operator=( atomic_ptr_t&& other ) noexcept { 
        mve( type::move( other ) ); return *this; 
    }
    
    /*─······································································─*/

    atomic_ptr_t( const atomic_ptr_t&  other ) noexcept : obj(nullptr) { cpy( other ); }
    atomic_ptr_t& operator=( const atomic_ptr_t& other ) noexcept { 
        cpy( other ); return *this; 
    }

    /*─······································································─*/

    ulong count() const noexcept { return(obj==nullptr) ? 0UL : obj->count.get(); }
    bool  null () const noexcept { return obj==nullptr; }

    /*─······································································─*/

    T* operator->() const noexcept { return  data(); }
    T* operator& () const noexcept { return  data(); }
    T& operator* () const noexcept { return *data(); }
    
    /*─······································································─*/

    T* get () const noexcept { return (obj==nullptr) ? nullptr : obj->value; }
    T* data() const noexcept { return (obj==nullptr) ? nullptr : obj->value; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/