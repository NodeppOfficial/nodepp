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

namespace nodepp { 
template< class T, class = typename type::enable_if<type::is_trivially_copyable<T>::value,T>::type >
class atomic_t   { private: volatile T value; protected: 

    void cpy( const atomic_t& other ) noexcept { memcpy ( (void*)&value, (void*)&other.value, sizeof( T ) ); }

    void mve( atomic_t&& other ) /**/ noexcept { memmove( (void*)&value, (void*)&other.value, sizeof( T ) ); }

public:

    atomic_t( atomic_t&& other ) noexcept { mve(type::move(other)); }

    atomic_t( const atomic_t& other ) noexcept { cpy(other); }

    atomic_t( T _val_ ) noexcept : value( _val_ ) {}

    atomic_t() noexcept : value( T{} ) {}

public:

    bool compare( T& expected, T desired ) const noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return (T)( ::InterlockedCompareExchange  ( (LONG   volatile *)&value, (LONG)  desired, (LONG)  expected ) ); break;
        case 8 : return (T)( ::InterlockedCompareExchange64( (LONG64 volatile *)&value, (LONG64)desired, (LONG64)expected ) ); break;
    }}

    T swap( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return (T)( ::InterlockedExchange  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return (T)( ::InterlockedExchange64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    /*─······································································─*/

    void set( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: (T)( ::InterlockedExchange  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : (T)( ::InterlockedExchange64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    T get() const noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return (T)( ::InterlockedExchangeAdd  ( (LONG   volatile *)&value, 0 ) ); break;
        case 8 : return (T)( ::InterlockedExchangeAdd64( (LONG64 volatile *)&value, 0 ) ); break;
    }}

    /*─······································································─*/

    T _or( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return (T)( ::InterlockedOr  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return (T)( ::InterlockedOr64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    T _and( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return (T)( ::InterlockedAnd  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return (T)( ::InterlockedAnd64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    T _xor( T new_val ) noexcept { switch( sizeof( T ) ){
        case 2 : case 4 :
        default: return (T)( ::InterlockedXor  ( (LONG   volatile *)&value, (LONG)  new_val ) ); break;
        case 8 : return (T)( ::InterlockedXor64( (LONG64 volatile *)&value, (LONG64)new_val ) ); break;
    }}

    /*─······································································─*/

    T add( T new_val ) noexcept { 
        size_t scale = 1; if ( type::is_pointer<T>::value ) 
             { scale = sizeof( typename type::remove_pointer<T>::type ); }
    switch( sizeof( T ) ){
        case 2 : case 4 : 
        default: return (T)( ::InterlockedExchangeAdd  ( (LONG   volatile *)&value, (LONG)  new_val * scale ) ); break;
        case 8 : return (T)( ::InterlockedExchangeAdd64( (LONG64 volatile *)&value, (LONG64)new_val * scale ) ); break;
    }}

    T sub( T new_val ) noexcept { 
        size_t scale = 1; if ( type::is_pointer<T>::value ) 
             { scale = sizeof( typename type::remove_pointer<T>::type ); }
    switch( sizeof( T ) ){
        case 2 : case 4 : 
        default: return (T)( ::InterlockedExchangeAdd  ( (LONG   volatile *)&value, -(LONG)  new_val * scale ) ); break;
        case 8 : return (T)( ::InterlockedExchangeAdd64( (LONG64 volatile *)&value, -(LONG64)new_val * scale ) ); break;
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

namespace nodepp { namespace atomic {

    inline void acquire(){ ::MemoryBarrier(); }
    inline void release(){ ::MemoryBarrier(); }
    
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