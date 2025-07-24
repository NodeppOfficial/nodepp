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
public:

    ptr_t( ulong n, const T& value ) noexcept { resize( n, value ); }
    ptr_t( T* value, ulong n )       noexcept { resize( value, n ); }
    ptr_t( T* value )                noexcept { resize( value ); }
    ptr_t( ulong n )                 noexcept { resize( n ); }

    template < class V, ulong N >
    ptr_t( const V (&value)[N] ) noexcept {
        reset(); resize(N); type::copy( value, value+N, begin() );
    }

    ptr_t() noexcept {                reset(); }
   ~ptr_t() noexcept { if( !null() ){ reset(); } }

    /*─······································································─*/

    ptr_t& operator=( ptr_t&& other ) noexcept {
       if( data() != &other ){ reset(); mve(type::move(other)); } return *this;
    }

    ptr_t& operator=( const ptr_t& other ) noexcept {
       if( data() != &other ){ reset(); cpy(other); } return *this;
    }

    ptr_t( ptr_t&& other ) noexcept {
       if( data() != &other ){ reset(); mve(type::move(other)); }
    }

    ptr_t( const ptr_t& other ) noexcept {
       if( data() != &other ){ reset(); cpy(other); }
    }

    /*─······································································─*/

    bool operator> ( ptr_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( ptr_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( ptr_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( ptr_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( ptr_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( ptr_t& oth ) const noexcept { return data()!=oth.data(); }

    /*─······································································─*/

    bool operator> ( T* value )   const noexcept { return data()> value; }
    bool operator>=( T* value )   const noexcept { return data()>=value; }
    bool operator< ( T* value )   const noexcept { return data()< value; }
    bool operator<=( T* value )   const noexcept { return data()<=value; }
    bool operator==( T* value )   const noexcept { return data()==value; }
    bool operator!=( T* value )   const noexcept { return data()!=value; }

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept {
       return size() !=0 ? data()[i%size()] : data()[i];
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
         reset(); resize(N); type::copy( value, value+N, begin() );
    }

    void resize( ulong n, const T& c ) noexcept {
         reset(); resize(n); fill(c);
    }

    void resize( ulong n ) noexcept { reset();
        if( n == 0 ){
            free_  = new bool( false ); 
            value_ = new T();
            length_= new ulong( n );
            count_ = new ulong( 1 ); used_ = true;
        } else {
            free_  = new bool( false ); 
            value_ = new T[n];
            length_= new ulong( n );
            count_ = new ulong( 1 ); used_ = true;
        }
    }

    void resize( T* c, ulong n ) noexcept {
        if( c == nullptr ){ return; } reset();
        free_  = new bool( false ); 
        length_= new ulong( n );
        count_ = new ulong( 1 );
        value_ = c; used_ =true;
    }

    void resize( T* c ) noexcept {
        if( c == nullptr ){ return; } reset();
        free_  = new bool( false ); 
        length_= new ulong( 0 );
        count_ = new ulong( 1 );
        value_ = c; used_ =true;
    }

    /*─······································································─*/

    void reset() noexcept {

        if( null () )    { return; }
        if( count() !=0 && !null() ){
        if( --(*count_)==0 ){
            if(*length_==0 ){
                     delete    data();
            } else { delete [] data(); }
                     delete     free_;
                     delete    count_;
                     delete   length_;
        }}

        length_= nullptr;
        count_ = nullptr;
        value_ = nullptr;
        free_  = nullptr; used_ = false;
    }

    /*─······································································─*/

    ulong    count() const noexcept { return null() ? 0 : ( _EXIT_||*free_ ) ? 1 : *count_ ; }
    T*         end() const noexcept { return null() ? nullptr : value_+size(); }
    T*       begin() const noexcept { return null() ? nullptr : value_; }
    ulong     size() const noexcept { return null() ? 0 : *length_; }
    bool      null() const noexcept { return used_ ==false; }

    void      free() const noexcept {    if( !null() ){ * free_ = 1; } }
    bool     empty() const noexcept { return  null() ||  size()== 0; }
    bool has_value() const noexcept { return !null() && count()!= 0; }

    T*        data() const noexcept { return  begin(); }
    T*         get() const noexcept { return  begin(); }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return  has_value(); }
    explicit operator   T*(void) const          { return  data(); }

    T* operator->()              const noexcept { return  data(); }
    T& operator* ()              const noexcept { return *data(); }
    T* operator& ()              const noexcept { return  data(); }

    /*─······································································─*/

protected:

    ulong* length_ = nullptr;
    ulong*  count_ = nullptr;
     bool*   free_ = nullptr;
        T*  value_ = nullptr;
    bool     used_ = false  ;

    /*─······································································─*/

    void cpy( const ptr_t& other ) noexcept {
         if ( other.null() ){ return; }
         length_ = other.length_;
         count_  = other. count_; used_ = true;
         value_  = other. value_; 
         free_   = other.  free_; ++(*count_ );
    }

    void mve( ptr_t&& other ) noexcept {
         if ( other.null() ){ return; }
         length_ = other.length_;
         count_  = other .count_;
         value_  = other .value_;
         free_   = other .free_ ; this->used_ = true;
         other.  free_ = nullptr;
         other.length_ = nullptr;
         other. count_ = nullptr; 
         other. value_ = nullptr; other.used_ = false;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif