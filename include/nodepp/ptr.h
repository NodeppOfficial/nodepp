/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_PTR
#define NODEPP_PTR

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class ptr_t { 
public:
    
    ptr_t( ulong n, const T& value ) noexcept { resize( n, value ); }
    ptr_t( T* value, ulong n )       noexcept { resize( value, n ); }
    ptr_t( T* value )                noexcept { resize( value, 0 ); }
    ptr_t( ulong n )                 noexcept { resize( n ); }
    ptr_t()                          noexcept { reset(); }

    virtual ~ptr_t() noexcept { reset(); }
    
    /*─······································································─*/

    ptr_t( ptr_t&& other ) noexcept { mve( type::move(other) ); }
    
    ptr_t( const ptr_t& other ) noexcept { cpy(other); }

    template < class V, ulong N > 
    ptr_t ( const V (&value)[N] ) noexcept { 
        ulong s = 0; this->resize( N ); 
        for( auto x=begin(); x!=end(); x++ )
           { (*x) = (T)value[s]; s++; } 
    }
    
    /*─······································································─*/

    bool operator> ( ptr_t& oth ) const noexcept { return this->value_> oth.value_; }
    bool operator>=( ptr_t& oth ) const noexcept { return this->value_>=oth.value_; }
    bool operator< ( ptr_t& oth ) const noexcept { return this->value_< oth.value_; }
    bool operator<=( ptr_t& oth ) const noexcept { return this->value_<=oth.value_; }
    bool operator==( ptr_t& oth ) const noexcept { return this->value_==oth.value_; }
    bool operator!=( ptr_t& oth ) const noexcept { return this->value_!=oth.value_; }
    
    /*─······································································─*/

    bool operator> ( T* value )   const noexcept { return this->value_> value; }
    bool operator>=( T* value )   const noexcept { return this->value_>=value; }
    bool operator< ( T* value )   const noexcept { return this->value_< value; }
    bool operator<=( T* value )   const noexcept { return this->value_<=value; }
    bool operator==( T* value )   const noexcept { return this->value_==value; }
    bool operator!=( T* value )   const noexcept { return this->value_!=value; }
    
    /*─······································································─*/
    
    void fill( const T& c ) const noexcept {
        if( size() != 0 ) for( auto x=begin(); x!=end(); x++ ) (*x) = c;
        else              *value_ = c;
    }
    
    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept { 
        return size()==0 ? value_[i] : 
               value_[ i%size() ]; 
    }
    
    /*─······································································─*/

    ptr_t& operator=( const ptr_t& other ) noexcept {
        if( this->value_ != &other )
          { reset(); cpy(other); }
        return *this;
    }

    ptr_t& operator=( ptr_t&& other ) noexcept {
        if( this->value_ != &other )
          { reset(); mve(type::move(other)); }
        return *this;
    }
    
    /*─······································································─*/

    ptr_t copy() const noexcept {
          if( count() > 0 && size() == 0 )
            { return new T( *value_ ); }
        elif( count() > 0 && size() > 0 ){
            auto n_buffer = ptr_t<T>( size() );
            memcpy( &n_buffer, value_, size() );
            return n_buffer;
        }   return nullptr;
    }
    
    /*─······································································─*/

    void resize( ulong n, const T& c ) noexcept { reset(); 
        if( n == 0 ){ 
            length_= new ulong( 0 ); 
            count_ = new ulong( 1 );
            value_ = new T(c); return; 
        } else {
            length_= new ulong( n ); 
            count_ = new ulong( 1 );
            value_ = new T[n];
        }   while( n-->0 ) value_[n] = c;
    }

    void resize( T* c, ulong n ) noexcept { reset();
        if( c == nullptr ){ return; }
        length_= new ulong( n ); 
        count_ = new ulong( 1 );
        value_ = c;
    }
    
    void resize( ulong n ) noexcept { reset(); 
        if( n == 0 ){ return; }
        length_= new ulong( n ); 
        count_ = new ulong( 1 );
	    value_ = new T[n];
    }
    
    /*─······································································─*/

    void reset() noexcept {
        if( value_ == nullptr ){ return; }

        if( count() != 0 )      {
        if( --(*count_) == 0 )  {
            if(*length_ == 0 )  {
                     delete    value_;
            } else { delete [] value_; }
                     delete    count_;
                     delete   length_;
        }}

        length_= nullptr;
        count_ = nullptr;
        value_ = nullptr;
    }

    /*─······································································─*/

    bool has_value() const noexcept { return !null() && count()!= 0; }
    ulong     size() const noexcept { return  null() ? 0 : *length_; }
    ulong    count() const noexcept { return  null() ? 0 : *count_;  }
    bool     empty() const noexcept { return  null() || size()==0 ;  }
    bool      null() const noexcept { return  value_ == nullptr; }
    T*        data() const noexcept { return  value_; }
    T*         get() const noexcept { return  value_; }
    
    /*─······································································─*/

    T*    end() const noexcept { return value_ + size(); }
    T*  begin() const noexcept { return value_; }
    void free() const noexcept { *count_ = 0; }
    
    /*─······································································─*/

    explicit operator bool(void) const { return has_value(); }
    explicit operator   T*(void) const { return value_; }

    T* operator->() const noexcept { return  value_; }
    T& operator*()  const noexcept { return *value_; }
    T* operator&()  const noexcept { return  value_; }

    /*─······································································─*/

protected:

    ulong* length_= nullptr;
    ulong* count_ = nullptr;
        T* value_ = nullptr;
    
    /*─······································································─*/

    void cpy( const ptr_t& other ) noexcept {
        if( other.count() == 0 ){ return; }
        length_= other.length_;
        count_ = other.count_;
        value_ = other.value_; ++(*count_);
    }
    
    /*─······································································─*/

    void mve( ptr_t&& other ) noexcept {
        if( other.count() == 0 ){ return; }
        length_ = other.length_;
        count_  = other.count_;
        value_  = other.value_;
        other.length_= nullptr;
        other.count_ = nullptr;
        other.value_ = nullptr;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif