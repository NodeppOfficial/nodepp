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

    static constexpr ulong SSO = ( STACK_SIZE>0 && type::is_trivially_copyable<T>::value ) ? STACK_SIZE : 1;

#ifndef NODEPP_ATOMIC_SMART_POINTER_OFF

    struct NODE_STACK {
        atomic_t<ulong> /*------*/ count; 
        ulong length; T* value; int flag;
        alignas(T) char stack [SSO];
    };

    struct NODE_HEAP {
        atomic_t<ulong>  count; 
        ulong length; T* value; 
        void* stack; int  flag; 
    };

#else

    struct NODE_STACK {
        ulong /*----------------*/ count; 
        ulong length; T* value; int flag;
        alignas(T) char stack [SSO];
    };

    struct NODE_HEAP {
        ulong /*------*/ count; 
        ulong length; T* value; 
        void* stack; int  flag; 
    };

#endif

    enum FLAG {
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

    #ifndef NODEPP_ATOMIC_SMART_POINTER_OFF
        if( address->count.sub(1) == 1 )
          { _free_(address); delete address; }
    #else
        if( address->count  --    == 1 )
          { _free_(address); delete address; }
    #endif

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
    #ifndef NODEPP_ATOMIC_SMART_POINTER_OFF
        output = address; address->count.add(1);
    #else
        output = address; address->count++;
    #endif        
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

    void cpy( const ptr_t& other ) noexcept {
         reset(); _cpy_( other.address, address );
         offset = other.offset;
         limit  = other.limit ;
    }

    void mve( ptr_t&& other ) noexcept {
         reset(); _mve_( other.address, address );
         offset = other.offset;
         limit  = other.limit ;
    }

public:

    ptr_t& operator=( /*-*/ ptr_t&& other ) noexcept { mve(type::move(other)); return *this; }
    ptr_t& operator=( const     T&  value ) noexcept { resize( 0UL, value ); return *this; }
    ptr_t& operator=( const ptr_t&  other ) noexcept { cpy(other); return *this; }

    /*─······································································─*/

    ptr_t( /*-*/ ptr_t&& other ) noexcept { mve(type::move(other)); }
    ptr_t( const ptr_t&  other ) noexcept { cpy(other); }

    /*─······································································─*/

    ptr_t( const ptr_t<T>& value, ulong _offset, ulong _limit ) noexcept {
    cpy  ( value ); slice( _offset, _limit ); }

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

    ptr_t() noexcept { /*----*/ }
   ~ptr_t() noexcept { clear(); }

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept { 
       return !empty() && i<size() ? data()[i] : data()[i%size()];
    }

    /*─······································································─*/

    bool operator> ( T* value ) const noexcept { return data()> value; }
    bool operator>=( T* value ) const noexcept { return data()>=value; }
    bool operator< ( T* value ) const noexcept { return data()< value; }
    bool operator<=( T* value ) const noexcept { return data()<=value; }
    bool operator==( T* value ) const noexcept { return data()==value; }
    bool operator!=( T* value ) const noexcept { return data()!=value; }

    /*─······································································─*/

    bool operator> ( ptr_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( ptr_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( ptr_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( ptr_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( ptr_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( ptr_t& oth ) const noexcept { return data()!=oth.data(); }

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

    void slice( ulong _offset, ulong _limit ) noexcept {
        if( _offset > _limit  ){ limit=0, offset=0; return; }
        if( _null_( address ) ){ /*--------------*/ return; }
        limit =min( address->length, _limit  + offset );
        offset=min( address->length, _offset + offset ); 
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

    ulong    count() const noexcept { return null() ? 0 /*-*/ : (ulong) address->count; }
    ulong     size() const noexcept { return null() ? 0 /*-*/ : limit - offset; }
    
    T*       begin() const noexcept { return null() ? nullptr : _begin_( address ); }
    T*         end() const noexcept { return null() ? nullptr : _end_  ( address ); }

    bool     empty() const noexcept { return  null() ||  size() == 0; }
    bool has_value() const noexcept { return !null() && count() != 0; }
    bool      null() const noexcept { return _null_ ( address ); }

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

namespace nodepp { template< class T > class ref_t {
public:

    ref_t( ptr_t<T> value ) noexcept : address( new ptr_t<T> ){ *address=value; }
    ref_t() /*-----------*/ noexcept : address(){}
    ref_t( null_t ) /*---*/ noexcept {}

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept { 
       return !empty() && i<size() ? data()[i] : data()[i%size()];
    }

    /*─······································································─*/

    bool operator> ( T* value ) const noexcept { return data()> value; }
    bool operator>=( T* value ) const noexcept { return data()>=value; }
    bool operator< ( T* value ) const noexcept { return data()< value; }
    bool operator<=( T* value ) const noexcept { return data()<=value; }
    bool operator==( T* value ) const noexcept { return data()==value; }
    bool operator!=( T* value ) const noexcept { return data()!=value; }

    /*─······································································─*/

    bool operator> ( ref_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( ref_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( ref_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( ref_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( ref_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( ref_t& oth ) const noexcept { return data()!=oth.data(); }

    /*─······································································─*/

    ulong    count() const noexcept { return null() ? 0 /*-*/ : address->count(); }
    ulong     size() const noexcept { return null() ? 0 /*-*/ : address->size (); }

    T*        data() const noexcept { return null() ? nullptr : address->data(); }
    T*         get() const noexcept { return null() ? nullptr : address->data(); }
    T*       begin() const noexcept { return null() ? nullptr : address->data(); }
    T*         end() const noexcept { return null() ? nullptr : address->end (); }

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
    ptr_t<T> bind( const T& object ){ return ptr_t<T>( new T( object ) ); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif