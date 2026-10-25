/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ATOMIC
#define NODEPP_ATOMIC

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined( NODEPP_THREAD_SUPPORTED )
    #error "This OS Does not support atomic.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if   NODEPP_KERNEL == NODEPP_KERNEL_WINDOWS
    #include "windows/atomic.h"
#elif NODEPP_KERNEL == NODEPP_KERNEL_POSIX
    #include "posix/atomic.h"
#else
    #error "This OS Does not support atomic.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ATOMIC_PTR
#define NODEPP_ATOMIC_PTR

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, ulong STACK_SIZE = NODEPP_MAX_SSO_SIZE > class atomic_ptr_t {
private:

#if NODEPP_ALLOW_SSO == 1
    static constexpr ulong SSO = ( STACK_SIZE>0 && type::is_trivially_copyable<T>::value ) ? STACK_SIZE : 1;
#else 
    static constexpr ulong SSO = 1;
#endif

    struct NODE_STACK {
        atomic_t<ulong> count; ulong length; 
        T* value; uchar flag ; char stack [SSO];
    };

    struct NODE_HEAP {
        atomic_t<ulong> count; ulong length; 
        T* value; uchar flag ; char stack [1];
    };

#endif

    enum FLAG : uchar {
         PTR_FLAG_UNKNOWN = 0b00000000,
         PTR_FLAG_HEAP    = 0b00000001,
         PTR_FLAG_STACK   = 0b00000010,
         PTR_FLAG_USED    = 0b00000100
    };

    using NODE = typename type::conditional<( SSO==1 ),NODE_HEAP,NODE_STACK>::type;
    ulong offset=0, limit=0;

    /*─······································································─*/

    inline int _free_( NODE* address ) const noexcept {
        
        if  ( address /*-*/ ==nullptr ){ return -1; }
        if  ( address->value==nullptr ){ return -1; }
        if  ( address->flag ==0x00    ){ return -1; }

        if  ( address->flag & FLAG::PTR_FLAG_HEAP ){
        if  ( address->length!= 0 ){ delete [] address->value; }
        else /*-----------------*/ { delete    address->value; }}
        
        address->flag  = FLAG::PTR_FLAG_UNKNOWN;
        address->value = nullptr;
        address->count = 0UL;

    return 1; }

    inline int _del_( NODE*& address ) const noexcept {
        if( address == nullptr ){ return -1; }
        if( address->count ==0 ){ return -1; }

        if( address->count.sub(1) == 1 )
          { _free_(address); delete address; }

    address = nullptr; return 1; }

    /*─······································································─*/

    inline int _set_( NODE*& address, T* value, ulong N, bool ) noexcept {
        if( value == nullptr ) /*-*/ { return -1; }
        if( _set_( address, N )==-1 ){ return -1; }

        if( address->flag & FLAG::PTR_FLAG_STACK ){
            address->value= (T*)( address->stack );
        } else {
            if( N==0 ){ address->value = new T( ); }
            else      { address->value = new T[N]; }
        }

        if( N==0 ){ *address->value = *value; }
        else      { type::copy( value, value+N, begin() ); }
        
    return 1; }

    inline int _set_( NODE*& address, T* value, ulong N ) noexcept {
        if( value == nullptr ) /*-*/ { return -1; }
        if( _set_( address, N )==-1 ){ return -1; }

        address->flag &=~FLAG::PTR_FLAG_STACK;
        address->flag |= FLAG::PTR_FLAG_HEAP ;
        address->value = value;
        
    return 1; }

    inline int _set_( NODE*& address, ulong N, bool ) noexcept {
        if( _set_( address, N )==-1 ){ return -1; }

        if( address->flag & FLAG::PTR_FLAG_STACK ){
            address->value= (T*)( address->stack );
        } else {
            if( N==0 ){ address->value = new T( ); }
            else      { address->value = new T[N]; }
        }
    return 1; }

    inline int _set_( NODE*& address, ulong N ) noexcept {

        if( address           == nullptr )
          { return _new_ ( address, N ); }
        if( _del_( address )       == -1 )
          { address = nullptr; return -1;}
        if( _new_( address, N )    == -1 )
          { address = nullptr; return -1;}
        
    return 1; }

    /*─······································································─*/

    inline int _uno_( NODE*& address, const T& c ) noexcept {
        if( _set_( address, 0UL )==-1 ){ return -1; }

        if( address->flag & FLAG::PTR_FLAG_STACK ){
            address->value= (T*)( address->stack );
            memcpy( (void*) address->value, (void*)&c, sizeof(T) );
        } else { address->value = new T(c); }
        
    return 1; }

    /*─······································································─*/

    inline int _new_( NODE*& address, ulong N ) noexcept {
        if( address!= nullptr ){ return -1; }

        ulong r_size = (N == 0) ? sizeof(T) : (sizeof(T) * N);
        address      = new NODE(); offset=0; limit=N;

        address->count  = 1 ;
        address->length = N ;
        address->value  = nullptr;
        address->flag   = FLAG::PTR_FLAG_USED;

        if  ( r_size <= SSO && type::is_trivially_copyable<T>::value ){
              address->value= (T*)( address->stack );
              address->flag|= FLAG::PTR_FLAG_STACK; } 
        else{ address->flag|= FLAG::PTR_FLAG_HEAP ; }

    return 1; }

    /*─······································································─*/

    inline int _cpy_( NODE* address, NODE*& output ) const noexcept {
        if( _null_( address ) ){ return -1; }
        output = address; address->count.add(1);
    return 1; }

    inline int _mve_( NODE*& address, NODE*& output ) noexcept {
        if( _null_( address ) ){ return -1; }
        output = address; address = nullptr;
    return 1; }

    inline T* _begin_( NODE* address ) const noexcept {
        if(_null_( address ) ){ return nullptr; }
    return address->value + offset; } 

    inline T* _end_( NODE* address ) const noexcept {
        if(_null_( address ) ){ return nullptr; }
    return address->value + limit; }

    /*─······································································─*/

    inline bool _null_( NODE* address ) const noexcept {
        if( address /*-*/ ==nullptr ){ return true; }
        if( address->flag == 0x00   ){ return true; }
        if( address->count== 0UL    ){ return true; }
        if( address->value==nullptr ){ return true; }
    return false; }

private:
    
    NODE* address = nullptr;

protected:

    void cpy( const atomic_ptr_t& other ) noexcept {
         reset(); _cpy_( other.address, address );
         offset = other.offset;
         limit  = other.limit ;
    }

    void mve( atomic_ptr_t&& other ) noexcept {
         reset(); _mve_( other.address, address );
         offset = other.offset;
         limit  = other.limit ;
    }

    bool& shutdown() const noexcept { return NODEPP_SHTDWN(); }

public:

    atomic_ptr_t& operator=( /*-*/ atomic_ptr_t&& other ) noexcept { mve(type::move(other)); return *this; }
    atomic_ptr_t& operator=( const     T&  value ) noexcept { resize( 0UL, value ); return *this; }
    atomic_ptr_t& operator=( const atomic_ptr_t&  other ) noexcept { cpy(other); return *this; }

    /*─······································································─*/

    atomic_ptr_t( const atomic_ptr_t<T>& value, ulong _offset, ulong _limit ) noexcept : address(nullptr) {
        cpy( value ); slice( _offset, _limit ); 
    }

    /*─······································································─*/

    atomic_ptr_t( /*-*/ atomic_ptr_t&& other ) noexcept : address(nullptr) { mve(type::move(other)); }
    atomic_ptr_t( const atomic_ptr_t&  other ) noexcept : address(nullptr) { cpy(other); }

    /*─······································································─*/

    atomic_ptr_t( ulong N, const T& value ) noexcept : address(nullptr) { resize( N, value ); }
    atomic_ptr_t( T* value, ulong N ) /*-*/ noexcept : address(nullptr) { resize( value, N ); }
    atomic_ptr_t( T* value ) /*----------*/ noexcept : address(nullptr) { resize( value ); }
    atomic_ptr_t( ulong N ) /*-----------*/ noexcept : address(nullptr) { resize( N ); }

    /*─······································································─*/

    template < class V, ulong N >
    atomic_ptr_t( const V (&value)[N] ) noexcept : address(nullptr) { 
        resize(N); type::copy( value, value+N, begin() ); 
    }

    /*─······································································─*/

    atomic_ptr_t() noexcept : address(nullptr) { /*----*/ }
   ~atomic_ptr_t() noexcept /*--------------*/ { clear(); }

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept { 
       return i<size() ? data()[i] : !empty() ? data()[i%size()] : data()[0];
    }

    /*─······································································─*/

    bool operator> ( T* value ) const noexcept { return data()> value; }
    bool operator>=( T* value ) const noexcept { return data()>=value; }
    bool operator< ( T* value ) const noexcept { return data()< value; }
    bool operator<=( T* value ) const noexcept { return data()<=value; }
    bool operator==( T* value ) const noexcept { return data()==value; }
    bool operator!=( T* value ) const noexcept { return data()!=value; }

    /*─······································································─*/

    bool operator> ( atomic_ptr_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( atomic_ptr_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( atomic_ptr_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( atomic_ptr_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( atomic_ptr_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( atomic_ptr_t& oth ) const noexcept { return data()!=oth.data(); }

    /*─······································································─*/

    atomic_ptr_t copy() const noexcept {
        if  ( _null_( address ) ){ return nullptr; }
        if  ( count() > 0 && size()==0 )
            { return new T( *data() ); }
        elif( count() > 0 && size()> 0 ){ 
              auto n_buffer=atomic_ptr_t<T>( size() );
              type::copy( begin(), end(), n_buffer.begin() );
        return n_buffer; } return nullptr;
    }

    atomic_ptr_t& slice( ulong _offset, ulong _limit ) noexcept {
        if( _null_( address ) ){ /*--------------*/ return *this; }
        if( _limit  > address->length ){ _limit =address->length; }
        if( _offset > _limit  ){ limit=0, offset=0; return *this; }
    limit = _limit > address->length ? address->length : min( address->length, _limit  + offset );
    offset= _offset> address->length ? address->length : min( address->length, _offset + offset ); 
    return *this; }

    /*─······································································─*/

    void resize( ulong N, const T& c ) noexcept {
         if( N==0UL ){ _uno_( address, c ); } else { resize(N); fill(c); }
    }

    template < class V, ulong N >
    void resize( const V (&value)[N] ) noexcept {
         resize(N); type::copy( value, value+N, begin() );
    }

    void resize( ulong N ) noexcept {
         _set_( address, N, false );
    }

    void resize( T* c, ulong N ) noexcept {
         if( c==nullptr ){ _del_( address ); }
         else /*------*/ { _set_( address, c, N ); }
    }

    void resize( T* c ) noexcept { resize( c, 0UL ); }

    /*─······································································─*/

    template < class V, ulong N >
    void fill( const V (&value)[N] ) const noexcept {
        if  ( empty() ){ return; }
        if  ( type::is_trivially_copyable<V>::value )
            { memcpy( begin(), value, N*sizeof(T) ); }
        else{ type::copy( value, value+N, begin() ); }
    }

    void fill( T* value, ulong N ) const noexcept { 
        if  ( empty() || value == nullptr ){ return; }
        if  ( type::is_trivially_copyable<T>::value )
            { memcpy( begin(), value, N*sizeof(T) ); }
        else{ type::copy( value, value+N, begin() ); }
    }

    void fill( const T& value ) const noexcept {
        if  ( null()    ){ return; }
        if  ( size()==0 ){ *data() = value; }
        else{ type::fill( begin(), end(), value ); }
    }

    /*─······································································─*/

    void clear() noexcept { _del_( address ); }
    void reset() noexcept { _del_( address ); }

    /*─······································································─*/

    ulong    count() const noexcept { return null() ? 0 : shutdown() ? 1 : (ulong) address->count; }
    ulong     size() const noexcept { return null() ? 0 : limit - offset; }

    bool     empty() const noexcept { return  null() ||  size() == 0; }
    bool has_value() const noexcept { return !null() && count() != 0; }
    bool      null() const noexcept { return _null_ ( address ); }
    
    T*       begin() const noexcept { return _begin_( address ); }
    T*         end() const noexcept { return _end_  ( address ); }

    T*        data() const noexcept { return _begin_( address ); }
    T*         get() const noexcept { return _begin_( address ); }

    void      free() const noexcept { _free_( address ); }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return  has_value(); }
    explicit operator   T*(void) const /*----*/ { return  data(); }

    T* operator->() /*--------*/ const noexcept { return  data(); }
    T& operator* () /*--------*/ const noexcept { return *data(); }
    T* operator& () /*--------*/ const noexcept { return  data(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/