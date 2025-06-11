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

    ptr_t<ulong> get_slice_range( long x, long y ) const noexcept {

        if( empty() || x == y ){ return nullptr; } if( y>0 ){ y--; }

        if( x < 0 ){ x = size() + x; } if( (ulong)x > last() ){ return nullptr; }
        if( y < 0 ){ y = last() + y; } if( (ulong)y > last() ){ y = last(); }
        if( y < x ){ return nullptr; }

        ulong a = clamp( first() + y, 0UL, last() );
        ulong b = clamp( first() + x, 0UL, a      );
        ulong c = a - b + 1;

        return ptr_t<ulong>({ b, a, c });
    }

    ptr_t<ulong> get_splice_range( long x, ulong y ) const noexcept {

        if( empty() || y == 0 ){ return nullptr; }

        if( x < 0 ){ x = last() + x; } if( (ulong)x > last() ){ return nullptr; }
            y += x - 1;
        if( y > last() ){ y= last(); } if( y < (ulong)x ){ return nullptr; }

        ulong a = clamp( first() + y, 0UL, last() );
        ulong b = clamp( first() + x, 0UL, a      );
        ulong c = a - b + 1;

        return ptr_t<ulong>({ b, a, c });
    }

public: array_t() noexcept {};

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
        ulong s = 0; buffer = ptr_t<T>( N );
        for( auto x=begin(); x!=end(); x++ )
           { (*x) = (T)value[s]; s++; }
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

    bool operator> ( const array_t& oth ) const noexcept { return compare( oth ) == 1; }
    bool operator>=( const array_t& oth ) const noexcept { return compare( oth ) >= 0; }
    bool operator<=( const array_t& oth ) const noexcept { return compare( oth ) <= 0; }
    bool operator< ( const array_t& oth ) const noexcept { return compare( oth ) ==-1; }

    /*─······································································─*/

    bool operator==( const array_t& oth ) const noexcept { return compare( oth ) == 0; }
    bool operator!=( const array_t& oth ) const noexcept { return compare( oth ) != 0; }

    T& operator[]( ulong n ) const noexcept { return buffer[n]; }

    /*─······································································─*/

    long index_of( function_t<bool,T> func ) const noexcept { long i=0;
        for( auto& x : *this ){ if( func(x) ) return i; i++; } return -1;
    }

    ulong count( function_t<bool,T> func ) const noexcept { ulong n=0;
        for( auto& x : *this ){ if( func(x) ) n++; } return n;
    }

    /*─······································································─*/

    T reduce( function_t<T,T,T> func ) const noexcept { T act = (*this)[0];
        for( auto x=this->begin() + 1; x != this->end(); x++ )
           { act = func( act, *x ); } return act;
    }

    bool some( function_t<bool,T> func ) const noexcept {
        for( auto& x : *this ){ if( func(x)==1 ) return 1; } return 0;
    }

    bool none( function_t<bool,T> func ) const noexcept {
        for( auto& x : *this ){ if( func(x)==1 ) return 0; } return 1;
    }

    bool every( function_t<bool,T> func ) const noexcept {
        for( auto& x : *this ){ if( func(x)==0 ) return 0; } return 1;
    }

    void map( function_t<void,T&> func ) const noexcept {
        for( auto& x : *this ){ func(x); }
    }

    /*─······································································─*/

    ptr_t<int> find( const array_t& data, ulong offset=0 ) const noexcept {
        if ( data.empty() ){ return nullptr; }
        ulong x=0; int n=0; ptr_t<int> pos ({ 0, 0 });
        for( ulong i=offset; i<buffer.size(); i++ ){
            if ( buffer[i] == data[x] ){
                pos[n]=i; x++; n=1;
            } elif ( x==data.size() ){
                pos[n]=i; x++; n=1; break;
            } else { n=0; x=0; }
        }
        return !x ? nullptr : pos;
    }

    ptr_t<int> find( const T& data, ulong offset=0 ) const noexcept {
        return find( array_t( 1UL, data ), offset );
    }

    /*─······································································─*/

    int compare( const array_t& oth ) const noexcept {
        if( size() < oth.size() ){ return -1; }
        if( size() > oth.size() ){ return  1; }
        ulong n = size(); while( n-->0 ){
            if( (*this)[n] < oth[n] ){ return -1; }
            if( (*this)[n] > oth[n] ){ return  1; }
        }   return 0;
    }

    /*─······································································─*/

    array_t remove( function_t<bool,T> func ) noexcept {
        ulong n=size(); while( n-->0 ){ if( func((*this)[n]) ) erase(n); } return (*this);
    }

    array_t reverse() const noexcept { auto n_buffer = ptr_t<T>(size());
        ulong n=last(); for( auto& x : *this ){ n_buffer[n]=x; n--; } return n_buffer;
    }

    array_t replace( function_t<bool,T> func, const T& targ ) const noexcept {
        for( auto& x : *this ){ if(func(x)) x=targ; } return (*this);
    }

    array_t copy() const noexcept { return buffer.copy(); }

    /*─······································································─*/

    void fill( const char& argc ) const noexcept { buffer.fill(argc); }

    template< class... V >
    void resize( const V&... args ) noexcept { buffer.resize(args...); }

    /*─······································································─*/

    array_t sort( function_t<bool,T,T> func ) const noexcept {
        queue_t<T> n_buffer;

        for( ulong i=0; i<size(); i++ ){
            auto x = buffer[i]; auto n = n_buffer.first();
            while( n!=nullptr ){ if( !func( x, n->data ) )
                 { n = n->next; continue; } break;
            }      n_buffer.insert( n, x );
        }

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
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = ptr_t<T> ( 1 ); buffer[0] = value; }
        else { ulong n=size() + 1; auto n_buffer = ptr_t<T>(n);
            for( ulong x=0,i=0; x<n; x++ ){
                if( x == index ){ n_buffer[x] = value; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N, T* value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = ptr_t<T> ( value, N ); }
        else { ulong n=size() + N; auto n_buffer = ptr_t<T>(n);
            for( ulong x=0,i=0,p=0; x<n; x++ ){
                if( x>=index && x<index+N ){ n_buffer[x] = value[p++]; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N, const T& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = ptr_t<T> ( N, value ); }
        else { ulong n=size() + N; auto n_buffer = ptr_t<T>(n);
            for( ulong x=0,i=0; x<n; x++ ){
                if( x>=index && x<index+N ){ n_buffer[x] = value; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    void insert( ulong index, const array_t& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = ptr_t<T> ( value.size() );
            for( ulong i=0; i<value.size(); i++ ){ buffer[i] = value[i]; }
        } else { ulong n=size() + value.size(); auto n_buffer = ptr_t<T>(n);
            for( ulong x=0,i=0,p=0; x<n; x++ ){
                if( x>=index && x<index+value.size() ){ n_buffer[x] = value[p++]; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    template< class V, ulong N >
    void insert( ulong index, const V (&value)[N] ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = ptr_t<T> ( N );
            for( ulong i=0; i<N; i++ ){ buffer[i] = value[i]; }
        } else { ulong n=size() + N; auto n_buffer = ptr_t<T>(n);
            for( ulong x=0,i=0,p=0; x<n; x++ ){
             if( x>=index && x<index+N ){ n_buffer[x] = value[p++]; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    /*─······································································─*/

    void erase( ulong index ) noexcept {
	    auto r = get_slice_range( index, size() );
         if( r == nullptr ){ return; } else { auto n_buffer = ptr_t<T>( last() );
            for( ulong i=0, j=0; i<size() && !n_buffer.empty(); i++ ){
             if( i != r[0] ){ n_buffer[j] = buffer[i]; j++; }
            }   buffer = n_buffer;
        }
    }

    void erase( ulong start, ulong end  ) noexcept {
	    auto r = get_slice_range( start, end );
         if( r == nullptr ){ return; } else {
            if( r[0] >= size() || r[1] >= size() ) erase( last() );
            auto n_buffer = ptr_t<T>( size() - ( r[1] - r[0] ) - 1 );
            for( ulong i=0, j=0; i<size() && !n_buffer.empty(); i++ ){
             if( i < r[0] || i > r[1] ){ n_buffer[j] = buffer[i]; j++; }
            }   buffer = n_buffer;
        }
    }

    /*─······································································─*/

    string_t join( string_t c=", " ) const noexcept {
        if( empty() ){ return ""; } string_t result;
        for( auto x=begin(); x!=end(); x++ ){
            result += string::to_string(*x) + ((x==end()-1)?"":c);
        }   return result;
    }

    /*─······································································─*/

    array_t slice( long start ) const noexcept {

	    auto r = get_slice_range( start, size() );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = ptr_t<T>(r[2]); for( ulong x=r[0],y=0; x<=r[1]; x++ )
           { n_buffer[y++] = buffer[x]; } return n_buffer;
    }

    /*─······································································─*/

    array_t slice( long start, long end ) const noexcept {

	    auto r = get_slice_range( start, end );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = ptr_t<T>(r[2]); for( ulong x=r[0],y=0; x<=r[1]; x++ )
           { n_buffer[y++] = buffer[x]; } return n_buffer;
    }

    /*─······································································─*/

    array_t splice( long start, ulong end ) noexcept {

	    auto r = get_splice_range( start, end );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = ptr_t<T>(r[2]); for( ulong x=r[0],y=0; x<=r[1]; x++ )
           { n_buffer[y++] = buffer[x]; } erase( r[0], r[0]+end ); return n_buffer;
    }

    template< class V, ulong N >
    array_t splice( long start, ulong end, const V (&value)[N] ) noexcept {

	    auto r = get_splice_range( start, end );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = ptr_t<T>(r[2]);

        for( ulong x=r[0],y=0; x<=r[1]; x++ ){ n_buffer[y++]=buffer[x]; }
        erase( r[0], r[0]+end ); insert( r[0], value ); return n_buffer;

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

    array_t<string_t> split( string_t _str, char ch ){
        array_t<string_t> result; int c; while( !_str.empty() ){
            while((c=_str.index_of([=]( char c ){ return c==ch; }))==0 )
                 { _str.erase(0); continue; }
            if( c != -1 ){ result.push(_str.splice( 0, c )); }
            else         { result.push(_str); break; }
        }   return result;
    }

    /*─······································································─*/

    array_t<string_t> split( string_t _str, int ch ){
        ch = clamp( (ulong)ch, 1UL, _str.size() );
        array_t<string_t> result; while( !_str.empty() ){
            result.push( _str.splice( 0, ch ) );
        }   return result;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
