/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_PTR
#define NODEPP_PTR

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, ulong STACK_SIZE=MAX_SSO > class ptr_t {
private:

    struct NODE { atomic_t<ulong> count, length; T* value; char stack[STACK_SIZE]; int flag; };
    enum   FLAG {
           PTR_FLAG_UNKNOWN = 0b00000000,
           PTR_FLAG_HEAP    = 0b00000001,
           PTR_FLAG_STACK   = 0b00000010,
           PTR_FLAG_USED    = 0b00000100
    };

    /*─······································································─*/

    inline int _free_( NODE* address ) const noexcept {
        
        if  ( address /*-*/ ==nullptr ){ return -1; }
        if  ( address->value==nullptr ){ return -1; }
        if  ( address->flag ==0x00    ){ return -1; }

        if  ( address->flag & FLAG::PTR_FLAG_HEAP ){
        if  ( address->length!= 0 ){ delete [] address->value; }
        else /*-----------------*/ { delete    address->value; }}
        else{ address->value->~T(); }
        
        address->flag  = FLAG::PTR_FLAG_UNKNOWN;
        address->value = nullptr;
        address->count = 0UL;

    return 1; }

    /*─······································································─*/

    inline int _del_( NODE*& address ) const noexcept {
        if( address == nullptr ){ return -1; }
        if( address->count ==0 ){ return -1; }
          --address->count;

        if( address->count ==0 )
          { _free_(address); delete address; }

        address = nullptr;

    return 1; }

    /*─······································································─*/

    inline int _set_( NODE*& address, T* value, ulong N, bool ) noexcept {
        if( value == nullptr ) /*-*/ { return -1; }
        if( _set_( address, N )==-1 ){ return -1; }

        if( address->flag & FLAG::PTR_FLAG_STACK ){
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

    inline int _new_( NODE*& address, ulong N ) noexcept {
        if( address!= nullptr ){ return -1; }

        address = new NODE();

        address->count  = 1 ;
        address->length = N ;
        address->value  = nullptr;
        address->flag   = FLAG::PTR_FLAG_USED;
        ulong r_size    = (N == 0) ? sizeof(T) : (sizeof(T) * N);

        if  ( r_size <= STACK_SIZE && type::is_trivially_copyable<T>::value ){
              address->value= (T*)( address->stack ); 
              address->flag |= FLAG::PTR_FLAG_STACK; } 
        else{ address->flag |= FLAG::PTR_FLAG_HEAP ; }

    return 1; }

    /*─······································································─*/

    inline int _cpy_( NODE* address, NODE*& output ) const noexcept {
        if( _null_( address ) ){ return -1; }
        output = address; ++address->count;
    return 1; }

    inline int _mve_( NODE*& address, NODE*& output ) noexcept {
        if( _null_( address ) ){ return -1; }
        output = address; address = nullptr;
    return 1; }

    inline T* _get_( NODE* address ) const noexcept {
        if(_null_( address ) ){ return nullptr; }
    return address->value; } 

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

    void cpy( const ptr_t& other ) noexcept {
         reset(); _cpy_( other.address, address );
    }

    void mve( ptr_t&& other ) noexcept {
         reset(); _mve_( other.address, address );
    }

public:

    ptr_t& operator=( /*-*/ ptr_t&& other ) noexcept { mve(type::move(other)); return *this; }
    ptr_t& operator=( const ptr_t&  other ) noexcept { cpy(other); return *this; }

    /*─······································································─*/

    ptr_t( /*-*/ ptr_t&& other ) noexcept { mve(type::move(other)); }
    ptr_t( const ptr_t&  other ) noexcept { cpy(other); }

    /*─······································································─*/

    ptr_t( ulong N, const T& value ) noexcept { resize( N, value ); }
    ptr_t( T* value, ulong N ) /*-*/ noexcept { resize( value, N ); }
    ptr_t( T* value ) /*----------*/ noexcept { resize( value ); }
    ptr_t( ulong N ) /*-----------*/ noexcept { resize( N ); }

    /*─······································································─*/

    template < class V, ulong N >
    ptr_t( const V (&value)[N] ) noexcept
    /*-*/{ resize(N); type::copy( value, value+N, begin() ); }

    /*─······································································─*/

    /*----*/ ptr_t() noexcept { /*----*/ }
    virtual ~ptr_t() noexcept { clear(); }

    /*─······································································─*/

    bool operator> ( ptr_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( ptr_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( ptr_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( ptr_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( ptr_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( ptr_t& oth ) const noexcept { return data()!=oth.data(); }

    /*─······································································─*/

    bool operator> ( T* value ) const noexcept { return data()> value; }
    bool operator>=( T* value ) const noexcept { return data()>=value; }
    bool operator< ( T* value ) const noexcept { return data()< value; }
    bool operator<=( T* value ) const noexcept { return data()<=value; }
    bool operator==( T* value ) const noexcept { return data()==value; }
    bool operator!=( T* value ) const noexcept { return data()!=value; }

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept {
       return ( empty() || i<size() ) ? data()[i] : data()[i%size()];
    }

    /*─······································································─*/

    ptr_t copy() const noexcept {
        if  ( _null_( address ) ){ return nullptr; }
        if  ( count() > 0 && size()==0 )
            { return new T( *data() ); }
        elif( count() > 0 && size()> 0 ){ 
              auto n_buffer=ptr_t<T>( size() );
              type::copy( begin(), end(), n_buffer.begin() );
        return n_buffer; } return nullptr;
    }

    /*─······································································─*/

    template < class V, ulong N >
    void resize( const V (&value)[N] ) noexcept {
         resize(N); type::copy( value, value+N, begin() );
    }

    void resize( ulong N, const T& c ) noexcept {
         resize(N); fill(c);
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

    ulong     size() const noexcept { return null() ? 0 /*-*/ : address->length.get(); }
    ulong    count() const noexcept { return null() ? 0 /*-*/ : address->count .get(); }
    
    T*         end() const noexcept { return null() ? nullptr : data() + size(); }
    T*       begin() const noexcept { return null() ? nullptr : data() ; }
    void      free() const noexcept { _free_( address ); }

    T*        data() const noexcept { return _get_( address );  }
    T*         get() const noexcept { return _get_( address );  }

    bool     empty() const noexcept { return  null() ||  size() == 0; }
    bool has_value() const noexcept { return !null() && count() != 0; }
    bool      null() const noexcept { return _null_( address ); }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return  has_value(); }
    explicit operator   T*(void) const /*----*/ { return  data(); }

    T* operator->() /*--------*/ const noexcept { return  data(); }
    T& operator* () /*--------*/ const noexcept { return *data(); }
    T* operator& () /*--------*/ const noexcept { return  data(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class ref_t {
public:

    ref_t( ptr_t<T> value ) noexcept : address( new ptr_t<T> ){ *address=value; }
    ref_t() /*-----------*/ noexcept : address(){}
    ref_t( null_t ) /*---*/ noexcept {}
    virtual ~ref_t() /*--*/ noexcept {}

    /*─······································································─*/

    bool operator> ( ref_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( ref_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( ref_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( ref_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( ref_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( ref_t& oth ) const noexcept { return data()!=oth.data(); }

    /*─······································································─*/

    bool operator> ( T* value ) const noexcept { return data()> value; }
    bool operator>=( T* value ) const noexcept { return data()>=value; }
    bool operator< ( T* value ) const noexcept { return data()< value; }
    bool operator<=( T* value ) const noexcept { return data()<=value; }
    bool operator==( T* value ) const noexcept { return data()==value; }
    bool operator!=( T* value ) const noexcept { return data()!=value; }

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept {
       return i>size() ? data()[i%size()] : data()[i]; 
    }

    /*─······································································─*/

    ulong    count() const noexcept { return null() ? 0 /*-*/ : address->count(); }
    ulong     size() const noexcept { return null() ? 0 /*-*/ : address->size(); }

    T*        data() const noexcept { return null() ? nullptr : address->data(); }
    T*         get() const noexcept { return null() ? nullptr : address->data(); }
    T*       begin() const noexcept { return null() ? nullptr : address->data(); }
    T*         end() const noexcept { return null() ? nullptr : address->end();  }

    bool     empty() const noexcept { return  null() || address->empty(); }
    bool has_value() const noexcept { return !null() && address->has_value(); }
    bool      null() const noexcept { return address.null() || address->null(); }

    /*─······································································─*/

    void clear() noexcept { address->clear(); }
    void reset() noexcept { address->reset(); }
    void free () noexcept { address->free (); }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return  has_value(); }
    explicit operator   T*(void) const /*----*/ { return  data(); }

    T* operator->() /*--------*/ const noexcept { return  data(); }
    T& operator* () /*--------*/ const noexcept { return *data(); }
    T* operator& () /*--------*/ const noexcept { return  data(); }

protected:

    ptr_t<ptr_t<T>> address;

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace type {

    template< class T, class V > 
    T* cast( V* object ){ 
        if( object==nullptr ){ return nullptr; } 
        return (T*)( object ); 
    }

    template< class T, class V > 
    T* cast( ptr_t<V>& object ){
        if ( object.null() ){ return nullptr; } 
        return (T*)( object.get() ); 
    }

    template< class T, class V > 
    T cast( V object ){ return (T)( object ); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace type {

    template< class T >
    ptr_t<T> bind( T* object ){
        if ( object==nullptr ){ return nullptr; }
        return ptr_t<T>( new T( *object ) ); 
    }

    template<class T> 
    ptr_t<T> bind( ptr_t<T>& object ){ return object; }

    template<class T, ulong N> 
    ptr_t<T> bind( const T(&value)[N] ){ 
    ptr_t<T> out(N); 
        if  ( type::is_trivially_copyable<T>::value )
            { memcpy    ( out.begin(), (T*)value, N ); }
        else{ type::copy( value, value+N, out.begin() ); }
    return out; }

    template<class T> 
    ptr_t<T> bind( const T& object ) { return ptr_t<T>( new T( object ) ); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif