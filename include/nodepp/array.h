/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ARRAY
#define NODEPP_ARRAY

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T >
class array_t {
protected:

    ptr_t<T> buffer;

    type::optional<ulong[3]> get_slice_range( long x, long y ) const noexcept {

        if( empty() || x == y ){ return nullptr; } if( y>0 ){ --y; }

        if( x < 0 ){ x = size() + x; } if( (ulong)x > last() ){ return nullptr; }
        if( y < 0 ){ y = last() + y; } if( (ulong)y > last() ){ y = last(); }
        if( y < x ){ return nullptr; }

        ulong a = clamp( first() + y, 0UL, last() );
        ulong b = clamp( first() + x, 0UL, a      );
        ulong c = a - b + 1;

        ulong arr[3]; /*-----------------------*/
              arr[0] = b; arr[1] = a; arr[2] = c;

        return arr;
    }

    type::optional<ulong[3]> get_splice_range( long x, ulong y ) const noexcept {

        if( empty() || y == 0 ){ return nullptr; }

        if( x < 0 ){ x = last() + x; } if( (ulong)x > last() ){ return nullptr; }
            y += x - 1;
        if( y > last() ){ y= last(); } if( y < (ulong)x ){ return nullptr; }

        ulong a = clamp( first() + y, 0UL, last() );
        ulong b = clamp( first() + x, 0UL, a      );
        ulong c = a - b + 1;

        ulong arr[3]; /*-----------------------*/
              arr[0] = b; arr[1] = a; arr[2] = c;

        return arr;
    }

public: 

    array_t() noexcept {}

    virtual ~array_t() noexcept {}

    array_t( const ptr_t<T>& argc ) noexcept { buffer = argc; }

    array_t( const ulong& n, const T& c ) noexcept {
        if ( n == 0 ){ return; } buffer = ptr_t<T>( n, c );
    }

    array_t( T* value, const ulong& n=0 ) noexcept {
        if ( value == nullptr || n == 0 ){ return; }
        buffer = ptr_t<T>( value, n );
    }

    template < class V, ulong N >
    array_t( const V (&value)[N] ) noexcept {
        buffer = ptr_t<T>( N );
        type::copy( value, value+N, begin() );
    }

    /*─······································································─*/

    T*   end() const noexcept { return &buffer + size(); }
    T* begin() const noexcept { return &buffer; }

    /*─······································································─*/

    ulong first() const noexcept { return 0; }
    bool  empty() const noexcept { return buffer.empty(); }
    ulong  size() const noexcept { return empty() ? 0 : buffer.size() - 0; }
    ulong  last() const noexcept { return empty() ? 0 : buffer.size() - 1; }

    /*─······································································─*/

    array_t operator+=( const array_t& oth ){
        if( oth.empty() ){ return *this; } /*------------------*/
        auto slf=copy(); buffer.resize( slf.size() +oth.size() );
        type::copy( oth.begin(), oth.end(), begin()+slf.size() );
        type::copy( slf.begin(), slf.end(), begin() ); return *this;
    }

    /*─······································································─*/

    bool operator> ( const array_t& oth ) const noexcept { return compare( oth ) == 1; }
    bool operator>=( const array_t& oth ) const noexcept { return compare( oth ) >= 0; }
    bool operator<=( const array_t& oth ) const noexcept { return compare( oth ) <= 0; }
    bool operator< ( const array_t& oth ) const noexcept { return compare( oth ) ==-1; }

    /*─······································································─*/

    bool operator==( const array_t& oth ) const noexcept { return compare( oth ) == 0; }
    bool operator!=( const array_t& oth ) const noexcept { return compare( oth ) != 0; }

    T& operator[]( ulong n ) const noexcept { return buffer[n]; }

    /*─······································································─*/

    int index_of( function_t<bool,T> func ) const noexcept {
        int out=0; auto addr = begin(); while( addr != end() ){
            if( func( *addr ) ){ return out; }
        ++addr; ++out; } return -1;
    }

    int count( function_t<bool,T> func ) const noexcept {
        int out=0; auto addr = begin(); while( addr != end() ){
            if( func( *addr ) ){ ++out; }
        ++addr; } return -1;
    }

    /*─······································································─*/

    T reduce( function_t<T,T,T> func ) const noexcept {
        auto out = *begin(); auto addr = begin();
        while( addr != end() ){ ++addr;
             out = func( out, *addr );
        } return out;
    }

    bool some( function_t<bool,T> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==1 ){ return 1; }
        ++addr; } return 0;
    }

    bool none( function_t<bool,T> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==1 ){ return 0; }
        ++addr; } return 1;
    }

    bool every( function_t<bool,T> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==0 ){ return 0; }
        ++addr; } return 1;
    }

    void map( function_t<void,T&> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
        func(*addr); ++addr; }
    }

    /*─······································································─*/

    ptr_t<int> find( const array_t& data, ulong offset=0 ) const noexcept {
        if( data.empty() || empty() ){ return nullptr; } /*------*/

        int pos = min( offset, size() ); auto addr = begin() + pos;
        ptr_t<int> idx ({ pos, pos }); ulong x=0;

        while( addr != end() ){ ++pos;
           if( data.size()== x ){ break; }
         elif( *addr == data[x] ){ idx[1]=pos; ++x; }
         else{ idx[0]=pos; idx[1]=pos; x=0; }
        ++addr; }
        
        return idx[0]!=idx[1] ? idx : nullptr;
    }

    ptr_t<int> find( const T& data, ulong offset=0 ) const noexcept {
        return find( array_t( 1UL, data ), offset );
    }

    /*─······································································─*/

    int compare( const array_t& oth ) const noexcept {
        if( size() < oth.size() ){ return -1; }
        if( size() > oth.size() ){ return  1; }
        return type::compare( begin(), end(), oth.begin() );
    }

    /*─······································································─*/

    array_t replace( function_t<bool,T> func, const T& targ ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==1 ){ *addr = targ; }
        ++addr; } return (*this);
    }

    array_t reverse() const noexcept { auto n_buffer = copy();
        type::reverse( begin(), end(), n_buffer.begin() );
        return n_buffer;
    }

    array_t remove( function_t<bool,T> func ) noexcept {
        ulong n=size(); while( n-->0 ){ 
            if( func((*this)[n]) ){ erase(n); }
        } return (*this);
    }

    array_t copy() const noexcept { return buffer.copy(); }

    /*─······································································─*/

    void fill( const char& argc ) const noexcept { buffer.fill(argc); }

    template< class... V >
    void resize( const V&... args ) noexcept { buffer.resize(args...); }

    /*─······································································─*/

    array_t sort( function_t<bool,T,T> func ) const noexcept {
        queue_t<T> n_buffer; T* addr = begin();

        while( addr != end() ){
            auto x = *addr; auto n = n_buffer.first();
            while( n!=nullptr ){ if( !func( x, n->data ) )
                 { n = n->next; continue; } break;
            }      n_buffer.insert( n, x );
        ++addr; }

        return n_buffer.data();
    }

    /*─······································································─*/

    void unshift( const T& value ) noexcept { insert( first(), value ); }
    void    push( const T& value ) noexcept { insert( size(), value ); }
    void                   shift() noexcept { erase( first() ); }
    void                     pop() noexcept { erase( last() ); }

    /*─······································································─*/

    void clear() noexcept { buffer.reset(); }
    void erase() noexcept { buffer.reset(); }
    void  free() noexcept { buffer.reset(); }

    /*─······································································─*/

    void insert( ulong index, const T& value ) noexcept {
	    index = clamp( index, 0UL, size() ); if( empty() ){ buffer = ptr_t<T> (1); buffer[0] = value; }
        else { ulong n=size() + 1; auto n_buffer = ptr_t<T>( n );
            type::copy( begin()+index, end()        , n_buffer.begin()+index+1 );
            type::copy( begin()      , begin()+index, n_buffer.begin()         );
            n_buffer[index] = value; buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N, const T& value ) noexcept {
	    index = clamp( index, 0UL, size() ); if( empty() ){ buffer = ptr_t<T> ( N, value ); }
        else{ ulong n=size() + N; auto n_buffer = ptr_t<T>( n );
            type::copy( begin()+index         , end()                   , n_buffer.begin()+index+N );
            type::fill( n_buffer.begin()+index, n_buffer.begin()+index+N, value                    );
            type::copy( begin()               , begin()+index           , n_buffer.begin()         );
            buffer = n_buffer;
        }
    }

    void insert( ulong index, const array_t& value ) noexcept {
	    index = clamp( index, 0UL, size() ); if( empty() ){ buffer = ptr_t<T> ( value.size() );
            type::copy( value.begin(), value.end(), buffer.begin() );
        } else { auto n_buffer = ptr_t<T>( size() + value.size() );
                 ulong N=value.size();
            type::copy( begin()+index, end()        , n_buffer.begin()+index+N );
            type::copy( value.begin(), value.end()  , n_buffer.begin()+index   );
            type::copy( begin()      , begin()+index, n_buffer.begin()         );
            buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N, T* value ) noexcept {
	    index = clamp( index, 0UL, size() ); if( empty() ){ buffer = ptr_t<T> ( value, N ); }
        else { ulong n=size() + N; auto n_buffer = ptr_t<T>( n );
            type::copy( begin()+index, end()        , n_buffer.begin()+index+N );
            type::copy( value        , value+N      , n_buffer.begin()+index   );
            type::copy( begin()      , begin()+index, n_buffer.begin()         );
            buffer = n_buffer;
        }
    }

    template< class V, ulong N >
    void insert( ulong index, const V (&value)[N] ) noexcept {
	    index = clamp( index, 0UL, size() ); if( empty() ){ buffer = ptr_t<T> ( N ); 
            type::copy( value, value + N, buffer.begin() );
        } else { ulong n=size() + N; auto n_buffer = ptr_t<T>( n );
            type::copy( begin()+index, end()        , n_buffer.begin()+index+N );
            type::copy( value        , value + N    , n_buffer.begin()+index   );
            type::copy( begin()      , begin()+index, n_buffer.begin()         );
            buffer = n_buffer;
        }
    }

    /*─······································································─*/

    void erase( ulong index ) noexcept {
	    auto r = get_slice_range( index, size() ); if( !r.has_value() ){ return; }
        else { auto z = *r.get(); auto n_buffer = ptr_t<T>( size() - 1 );
            type::copy( begin()+z[0]+1, end()       , n_buffer.begin()+z[0] );
            type::copy( begin()       , begin()+z[0], n_buffer.begin()      );
            buffer = n_buffer;
        }
    }

    void erase( ulong start, ulong stop ) noexcept {
	    auto r = get_slice_range( start, stop ); if( !r.has_value() ){ return; }
        else { auto z = *r.get(); auto n_buffer = ptr_t<T>( size() - z[2] );
            type::copy( begin()+z[1]+1, end()       , n_buffer.begin()+z[0] );
            type::copy( begin()       , begin()+z[0], n_buffer.begin()      );
            buffer = n_buffer;
        }
    }

    /*─······································································─*/

    string_t join( string_t c=", " ) const noexcept {
        if( empty() ){ return nullptr; } queue_t<string_t> borrow;
        /*---------------------------------*/ ulong size=0, off=0;

        auto addr = begin(); while( addr != end() ){
             borrow.push( string::to_string( *addr ) );
             size += borrow.last()->data.size();
        if( addr+1 != end() ){ size += c.size(); }
        ++addr; } 
        
        string_t out( size, 0x00 ); auto n=borrow.first(); 
        
        while( n!=nullptr ){
            type::copy( n->data.begin(), n->data.end(), out.begin()+off ); off+= n->data.size();
        if( off < out.size() )
          { type::copy( c.begin(), c.end(), out.begin()+off ); } /*-----*/ off+= c.size();
        n=n->next; }
        
        return out;
    }

    /*─······································································─*/

    array_t slice( long start ) const noexcept {

	    auto r = get_slice_range( start, size() );
        if( !r.has_value() ){ return nullptr; } 

        auto z = *r.get();
        auto n_buffer = ptr_t<T>( z[2] );
        type::copy( begin()+z[0], begin()+z[0]+z[2], n_buffer.begin() );
        
        return n_buffer;
    }

    /*─······································································─*/

    array_t slice( long start, long stop ) const noexcept {

	    auto r = get_slice_range( start, stop );
        if( !r.has_value() ){ return nullptr; } 

        auto z = *r.get();
        auto n_buffer = ptr_t<T>( z[2] );
        type::copy( begin()+z[0], begin()+z[0]+z[2], n_buffer.begin() );

        return n_buffer;
    }

    /*─······································································─*/

    array_t splice( long start, ulong stop ) noexcept {

	    auto r = get_splice_range( start, stop );
        if( !r.has_value() ){ return nullptr; } 

        auto z = *r.get();
        auto n_buffer = ptr_t<T>( z[2] );
        type::copy( begin()+z[0], begin()+z[0]+z[2], n_buffer.begin() );
        
        erase( z[0], z[0] + z[2] ); return n_buffer;
    }

    template< class V, ulong N >
    array_t splice( long start, ulong stop, const V (&value)[N] ) noexcept {

	    auto r = get_splice_range( start, stop );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get();
        auto n_buffer = ptr_t<T>( z[2] );
        type::copy( begin()+z[0], begin()+z[0]+z[2], n_buffer.begin() );

        erase( z[0], z[0]+z[2] ); insert( z[0], value ); return n_buffer;

    }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return empty(); }
    explicit operator T*(void) const noexcept { return &buffer; }

    const T* c_arr() const noexcept { return &buffer; }
          T*  data() const noexcept { return &buffer; }
          T*   get() const noexcept { return &buffer; }

    ptr_t<T>&  ptr() noexcept { return  buffer; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace string {
    
    template< class T >
    array_t<T> operator+( const array_t<T>& A, const array_t<T>& B ){
        if( A.empty() ){ return B; } if( B.empty() ){ return A; }
        ptr_t<T> C( A.size() + B.size() ); /*--------------*/
        type::copy( B.begin(), B.end(), C.begin()+A.size() );
        type::copy( A.begin(), A.end(), C.begin() ); return C;
    }

    /*─······································································─*/

    template< class T >
    array_t<string_t> split( string_t _str, const T& pattern ){
        queue_t<string_t> out; ulong offset=0; ptr_t<int> idx;
        
        while( (idx=_str.find( pattern, offset )) != nullptr ){
            out.push( _str.slice( offset, idx[0] ) ); offset=idx[1];
        }   out.push( _str.slice( offset ) );

        return out.data();
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif