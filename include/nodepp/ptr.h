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

namespace nodepp { template< class T > class ptr_t {
private:

    struct NODE { ulong count, length; T* value; };

    inline int _free_( NODE* address ) const noexcept {
        if( address /*--*/ == nullptr ){ return -1; }
        if( address->value == nullptr ){ return -1; }

        if( address->length!= 0 ){ delete [] address->value; }
        if( address->length== 0 ){ delete    address->value; }
        
            address->count = 0; /*-*/
            address->value = nullptr;
    return 1; }

    inline int _del_( NODE*& address ) const noexcept {
        if( address == nullptr ){ return -1; }
        if( address->count ==0 ){ return -1; }
          --address->count;
        
        if( address->count == 0 )
          { _free_(address); delete address; }

        address = nullptr;

    return 1; }

    inline int _set_( NODE*& address, T* value, ulong size ) noexcept {
        if( value  == nullptr ){ return -1; /*----------------------*/ }
        if( address== nullptr ){ return _new_( address, value, size ); }

        if( _del_( address ) /*-------*/ ==-1 )
          { address = nullptr; return -1; }
        if( _new_( address, value, size )==-1 )
          { address = nullptr; return -1; }
        
    return 1; }

    inline int _new_( NODE*& address, T* value, ulong size ) noexcept {
        if( value  == nullptr ){ return -1; }
        if( address!= nullptr ){ return -1; }

        address = new NODE();

        address->value = value;
        address->length= size;
        address->count = 1;

    return 1; }

    inline int _cpy_( NODE* address, NODE*& output ) const noexcept {
        if( address /*--*/ == nullptr ){ return -1; }
        if( address->count == 0 /*-*/ ){ return -1; }
        if( address->value == nullptr ){ return -1; }
        output = address; ++address->count;
    return 1; }

    inline int _mve_( NODE*& address, NODE*& output ) noexcept {
        if( address /*--*/ == nullptr ){ return -1; }
        if( address->count == 0 /*-*/ ){ return -1; }
        if( address->value == nullptr ){ return -1; }
        output = address; address = nullptr;
    return 1; }

    inline bool _null_( NODE* address ) const noexcept {
        if( address /*-*/ ==nullptr ){ return true; }
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

    ptr_t( ulong n, const T& value ) noexcept { resize( n, value ); }
    ptr_t( T* value, ulong n ) /*-*/ noexcept { resize( value, n ); }
    ptr_t( T* value ) /*----------*/ noexcept { resize( value ); }
    ptr_t( ulong n ) /*-----------*/ noexcept { resize( n ); }

    /*─······································································─*/

    template < class V, ulong N >
    ptr_t( const V (&value)[N] ) noexcept
    /*-*/{ resize(N); type::copy( value, value+N, begin() ); }

    /*─······································································─*/

    /*----*/ ptr_t() noexcept { /*----*/ }
    virtual ~ptr_t() noexcept { clear(); }

    /*─······································································─*/

    ptr_t& operator=( ptr_t&& other ) noexcept { mve(type::move(other)); return *this; }

    ptr_t& operator=( const ptr_t& other ) noexcept { cpy(other); return *this; }

    /*─······································································─*/

    ptr_t( ptr_t&& other ) noexcept { mve(type::move(other)); }

    ptr_t( const ptr_t& other ) noexcept { cpy(other); }

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
       return size() !=0 ? data()[i%size()] 
       /*-------------*/ : data()[i]; 
    }

    /*─······································································─*/

    void fill( const T& c ) const noexcept {
        if( null() ){ return; } if( size() != 0 )
          { type::fill( begin(), end(), c );
          } else { *data() =c; }
    }

    ptr_t copy() const noexcept {
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

    void resize( ulong n, const T& c ) noexcept {
         resize(n); fill(c);
    }

    void resize( ulong n ) noexcept { reset();
        if( n == 0 )
        /**/ { _set_( address, new T() , 0 ); } 
        else { _set_( address, new T[n], n ); }
    }

    void resize( T* c, ulong n ) noexcept {
         if( c == nullptr ){ _del_( address ); }
         else /*--*/ { _set_( address, c, n ); }
    }

    void resize( T* c ) noexcept {
         if( c == nullptr ){ _del_( address ); }
         else /*--*/ { _set_( address, c, 0 ); }
    }

    /*─······································································─*/

    void clear() noexcept { _del_( address ); }
    void reset() noexcept { _del_( address ); }

    /*─······································································─*/

    ulong     size() const noexcept { return null() ? 0 /*-*/ : address->length; }
    ulong    count() const noexcept { return null() ? 0 /*-*/ : address->count;  }
    T*        data() const noexcept { return null() ? nullptr : address->value;  }
    T*         get() const noexcept { return null() ? nullptr : address->value;  }

    T*         end() const noexcept { return null() ? nullptr : data() + size(); }
    T*       begin() const noexcept { return null() ? nullptr : data() ; }
    void      free() const noexcept { _free_( address ); }

    bool     empty() const noexcept { return  null() ||  size()== 0; }
    bool has_value() const noexcept { return !null() && count()!= 0; }
    bool      null() const noexcept { return _null_(address); }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return  has_value(); }
    explicit operator   T*(void) const /*----*/ { return  data(); }

    T* operator->() /*--------*/ const noexcept { return  data(); }
    T& operator* () /*--------*/ const noexcept { return *data(); }
    T* operator& () /*--------*/ const noexcept { return  data(); }

    /*─······································································─*/

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif