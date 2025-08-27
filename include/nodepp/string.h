/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_STRING
#define NODEPP_STRING

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

namespace string {

    inline bool   is_hex( uchar c ){ return ((c>='0' && c<='9') ||(c>='A'  && c<='F' ) || ( c>='a' && c<='f'  ) ); }
    inline bool is_space( uchar c ){ return ( c==' ' || c=='\t' || c=='\n' || c=='\r'  ||   c=='\f'|| c=='\v' ); }
    inline bool is_alpha( uchar c ){ return ((c>='A' && c<='Z') ||(c>='a'  && c<='z' ) ); }
    inline bool is_graph( uchar c ){ return ( c>=33  && c<=126  && c!=' ' ); }
    inline bool is_lower( uchar c ){ return ( c>='a' && c<='z' ); }
    inline bool is_upper( uchar c ){ return ( c>='A' && c<='Z' ); }
    inline bool is_digit( uchar c ){ return ( c>='0' && c<='9' ); }
    inline bool is_print( uchar c ){ return ( c>=32  && c<=127 ); }
    inline bool is_contr( uchar c ){ return ( c< 32  || c==127 ); }
    inline bool  is_null( uchar c ){ return ( c=='\0'); }
    inline bool is_ascii( uchar c ){ return ( c<=127 ); }

    /*─······································································─*/

    ptr_t<char> buffer( ulong n ){
        if ( n == 0 ){ return nullptr; }
        auto b = ptr_t<char>( n+1,'\0' ); return b;
    }

    ptr_t<char> buffer( const char* c, ulong n ){
        if ( c == nullptr ){ return nullptr; }
        if ( n == 0 ){ return nullptr; }
        auto b = ptr_t<char>( n+1,'\0' );
        memcpy( &b, c, n ); return b;
    }

    ptr_t<char> buffer( ulong n, const char& c ){
        if ( n == 0 ){ return nullptr; }
        auto b = ptr_t<char>( n+1,'\0' );
        memset( &b, c, n ); return b;
    }

    /*─······································································─*/

    inline int char_code( uchar c ){ return (int) c; }

    /*─······································································─*/

    inline char to_upper( uchar c ){ return is_lower(c) ? ( c - 'a' + 'A' ) : c; }
    inline char to_lower( uchar c ){ return is_upper(c) ? ( c - 'A' + 'a' ) : c; }

    /*─······································································─*/

    inline bool is_alnum( uchar c ){ return ( is_alpha(c) || is_digit(c) ); }
    inline bool is_punct( uchar c ){ return ( is_print(c) && !is_space(c) && !is_alnum(c) ); }

}

/*────────────────────────────────────────────────────────────────────────────*/

class string_t {
protected:

    ptr_t<char> buffer;

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

    virtual ~string_t() noexcept {}

    string_t() noexcept { buffer.clear(); }

    string_t( const char* argc ) noexcept {
        if( argc == nullptr ){
            buffer.clear(); return;
        }   buffer = string::buffer( argc, strlen(argc) );
    }

    string_t( const ulong& n, const char& c ) noexcept {
        if( n == 0 ){
            buffer.clear(); return;
        }   buffer = string::buffer( n, c );
    }

    string_t( const char* argc, const ulong& n ) noexcept {
        if( argc == nullptr || n == 0 ){
            buffer.clear(); return;
        }   buffer = string::buffer( argc, n );
    }

    /*─······································································─*/

    string_t( const ptr_t<char>& argc ) noexcept { buffer = argc; }

    /*─······································································─*/

    char*   end() const noexcept { return &buffer + size(); }
    char* begin() const noexcept { return &buffer; }

    /*─······································································─*/

    ulong first() const noexcept { return 0; }
    bool  empty() const noexcept { return buffer.size() <= 1; }
    ulong  size() const noexcept { return empty() ? 0 : buffer.size() - 1; }
    ulong  last() const noexcept { return empty() ? 0 : buffer.size() - 2; }

    /*─······································································─*/

    string_t operator+=( const string_t& oth ) noexcept {
        if( oth.empty() ){ return *this; } auto slf=copy();
        buffer = string::buffer( slf.size() + oth.size() );
        memcpy( begin()+slf.size () ,oth.begin(), oth.size() ); 
        memcpy( begin(),slf.begin() ,slf.size () ); return *this;
    }

    /*─······································································─*/

    bool operator> ( const string_t& oth ) const noexcept { return compare( oth ) == 1; }
    bool operator>=( const string_t& oth ) const noexcept { return compare( oth ) >= 0; }
    bool operator<=( const string_t& oth ) const noexcept { return compare( oth ) <= 0; }
    bool operator< ( const string_t& oth ) const noexcept { return compare( oth ) ==-1; }

    /*─······································································─*/

    bool operator==( const string_t& oth ) const noexcept { return compare( oth ) == 0; }
    bool operator!=( const string_t& oth ) const noexcept { return compare( oth ) != 0; }

    char& operator[]( ulong n ) const noexcept { return buffer[n]; }

    /*─······································································─*/

    int index_of( function_t<bool,char> func ) const noexcept {
        int out=0; auto addr = begin(); while( addr != end() ){
            if( func( *addr ) ){ return out; }
        ++addr; ++out; } return -1;
    }

    int count( function_t<bool,char> func ) const noexcept {
        int out=0; auto addr = begin(); while( addr != end() ){
            if( func( *addr ) ){ ++out; }
        ++addr; } return -1;
    }

    /*─······································································─*/

    char reduce( function_t<char,char,char> func ) const noexcept {
        auto out = *begin(); auto addr = begin();
        while( addr != end() ){ ++addr;
             out = func( out, *addr );
        } return out;
    }

    bool some( function_t<bool,char> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==1 ){ return 1; }
        ++addr; } return 0;
    }

    bool none( function_t<bool,char> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==1 ){ return 0; }
        ++addr; } return 1;
    }

    bool every( function_t<bool,char> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==0 ){ return 0; }
        ++addr; } return 1;
    }

    void map( function_t<void,char&> func ) const noexcept {
        auto addr = begin(); while( addr != end() ){
        func(*addr); ++addr; }
    }

    /*─······································································─*/

    ptr_t<int> find( const string_t& data, ulong offset=0 ) const noexcept {
        if( data.empty() || empty() ){ return nullptr; } /*------*/

        int pos = min( offset, size() ); auto addr = begin() + pos;
        ptr_t<int> idx ({ pos, pos }); ulong x=0;

        while( addr != end() ){ ++pos; 
           if( data.size() == x ){ break; }
         elif( *addr == data[x] ){ idx[1]=pos; ++x; }
         else{ idx[0]=pos; idx[1]=pos; x=0; }
        ++addr; }
        
        return idx[0]!=idx[1] ? idx : nullptr;
    }

    ptr_t<int> find( const char& data, ulong offset=0 ) const noexcept {
        return find( string_t( 1UL, data ), offset );
    }

    /*─······································································─*/

    int compare( const string_t& oth ) const noexcept {
        if( size() < oth.size() ){ return -1; }
        if( size() > oth.size() ){ return  1; }
        return memcmp( begin(), oth.begin(), size() );
    }

    /*─······································································─*/

    string_t replace( function_t<bool,char> func, char targ ) const noexcept {
        auto addr = begin(); while( addr != end() ){
            if( func(*addr)==1 ){ *addr = targ; }
        ++addr; } return (*this);
    }

    string_t reverse() const noexcept { auto n_buffer = copy();
        type::reverse( begin(), end(), n_buffer.begin() );
        return n_buffer;
    }

    string_t remove( function_t<bool,char> func ) noexcept {
        ulong n=size(); while( n-->0 ){ 
            if( func((*this)[n]) ){ erase(n); }
        } return (*this);
    }

    string_t copy() const noexcept { return buffer.copy(); }

    /*─······································································─*/

    void fill( const char& argc ) const noexcept { buffer.fill(argc); }

    template< class... T >
    void resize( T... args ) noexcept { buffer.resize(args...); }

    /*─······································································─*/

    string_t sort( function_t<bool,char,char> func ) const noexcept {
        queue_t<char> n_buffer; char* addr = begin();

        while( addr != end() ){
            auto x = *addr; auto n = n_buffer.first();
            while( n!=nullptr ){ if( !func( x, n->data ) )
                 { n = n->next; continue; } break;
            }      n_buffer.insert( n, x );
        ++addr; }  n_buffer.push('\0');

        return n_buffer.data();
    }

    /*─······································································─*/

    void unshift( char value ) noexcept { insert( first(), value ); }
    void    push( char value ) noexcept { insert( size(), value ); }
    void               shift() noexcept { erase( first() ); }
    void                 pop() noexcept { erase( last() ); }

    /*─······································································─*/

    void clear() noexcept { buffer.reset(); }
    void erase() noexcept { buffer.reset(); }
    void  free() noexcept { buffer.reset(); }

    /*─······································································─*/

    void insert( ulong index, const char& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer(1); buffer[0] = value; }
        else { ulong n=size() + 1; auto n_buffer = string::buffer(n);
            memcpy( &n_buffer+index+1, &buffer+index, buffer.size()-index );
            memcpy( &n_buffer,         &buffer      , index );
            n_buffer[index] = value; buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N , char* value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( value, N ); }
        else { ulong n=size() + N; auto n_buffer = string::buffer(n);
            memcpy( &n_buffer+index+N, &buffer+index, buffer.size()-index );
            memcpy( &n_buffer,         &buffer      , index );
            memcpy( &n_buffer+index  ,  value       , N );
            buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N , const char& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( N, value ); }
        else{ ulong n=size() + N; auto n_buffer = string::buffer(n);
            memcpy( &n_buffer+index+N, &buffer+index, buffer.size()-index );
            memcpy( &n_buffer,         &buffer      , index );
            memset( &n_buffer+index  ,  value       , N     );
            buffer = n_buffer;
        }
    }

    void insert( ulong index, const string_t& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( value.size() );
            memcpy( &buffer, value.begin(), value.size() );
        } else { auto n_buffer = string::buffer( size() + value.size() );
                 ulong N=value.size();
            memcpy( &n_buffer+index+N, &buffer+index, buffer.size()-index );
            memcpy( &n_buffer,         &buffer      , index );
            memcpy( &n_buffer+index  , value.begin(), N );
            buffer = n_buffer;
        }
    }

    template< ulong N >
    void insert( ulong index, const char (&value)[N] ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( N ); memcpy( &buffer, value, N );
        } else { ulong n=size() + N; auto n_buffer = string::buffer( n );
            memcpy( &n_buffer+index+N, &buffer+index, (buffer.size()-index) );
            memcpy( &n_buffer,         &buffer      , index );
            memcpy( &n_buffer+index  , &value       , N );
            buffer = n_buffer;
        }
    }

    /*─······································································─*/

    void erase( ulong index ) noexcept {
	    auto r = get_slice_range( index, size() );
        if( !r.has_value() ){ return; } else {
            auto z = *r.get(); auto n_buffer = string::buffer( size() - 1 );
            memcpy( &n_buffer+z[0], &buffer+z[0]+1, size()-z[0]-1 );
            memcpy( &n_buffer     , &buffer       , z[0] );
            buffer = n_buffer;
        }
    }

    void erase( ulong start, ulong stop  ) noexcept {
	    auto r = get_slice_range( start, stop );
        if( !r.has_value() ){ return; } else {
            auto z = *r.get(); auto n_buffer = string::buffer( size() - z[2] );
            memcpy( &n_buffer+z[0], &buffer+z[1]+1, size()-z[1]-1 );
            memcpy( &n_buffer     , &buffer       , z[0] );
            buffer = n_buffer;
        }
    }

    /*─······································································─*/

    string_t slice( long start ) const noexcept {

        auto r = get_slice_range( start, size() );
        if( !r.has_value() ){ return nullptr; }
        
        auto z = *r.get(); /*------------------------------*/
        auto n_buffer = string_t( buffer.data()+z[0], z[2] );
        
        return n_buffer;

    }

    /*─······································································─*/

    string_t slice( long start, long stop ) const noexcept {

        auto r = get_slice_range( start, stop );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); /*------------------------------*/
        auto n_buffer = string_t( buffer.data()+z[0], z[2] );

        return n_buffer;
    }

    /*─······································································─*/

    string_t splice( long start, ulong stop ) noexcept {

        auto r = get_splice_range( start, stop );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); /*------------------------------*/
        auto n_buffer = string_t( buffer.data()+z[0], z[2] );

        erase( z[0], z[0]+z[2] ); return n_buffer;
    }

    string_t splice( long start, ulong stop, string_t value ) noexcept {

        auto r = get_splice_range( start, stop );
        if( !r.has_value() ){ return nullptr; }

        auto z = *r.get(); /*------------------------------*/
        auto n_buffer = string_t( buffer.data()+z[0], z[2] );

        erase( z[0], z[0]+z[2] ); insert( z[0], value ); return n_buffer;
    }

    /*─······································································─*/

    string_t to_capital_case() const noexcept {
        if ( empty() ){ return nullptr; } bool b=1;
        auto out=string::buffer( size() );
        auto y=out.begin(); auto x=begin();
        while( x != end() ){
           if( string::is_alpha(*x) && b==1 ){ *y=string::to_upper(*x); b=0; goto DONE; }
           if(!string::is_alpha(*x) ){ b =1;}  *y=string::to_lower(*x);
        DONE:; ++x; ++y; } return out;
    }

    string_t to_slugify() const noexcept { if( empty() ){ return nullptr; } 
        auto out=string::buffer( size() ); ulong z=1; /*------*/
        auto y=out.begin(); auto x=begin(); while( x != end() ){ 
              if (!string::is_alnum(*x) ){ goto DONE; }
            else { *y = string::to_lower(*x); ++z; }
        DONE:; ++x; ++y; } return string_t( &out,z );
    }

    string_t to_lower_case() const noexcept { if( empty() ){ return nullptr; } 
        auto out=string::buffer( size() ); /*-----------------*/
        auto y=out.begin(); auto x=begin(); while( x != end() ){ 
            *y=string::to_lower(*x); 
        ++x; ++y; } return out;
    }

    string_t to_upper_case() const noexcept { if ( empty() ){ return nullptr; } 
        auto out=string::buffer( size() ); /*-----------------*/
        auto y=out.begin(); auto x=begin(); while( x != end() ){ 
            *y=string::to_upper(*x); 
        ++x; ++y; } return out;
    }

    /*─······································································─*/

    explicit operator char* (void) const noexcept { return empty() ? nullptr : &buffer; }
    explicit operator bool  (void) const noexcept { return empty(); }
    
          char*  data() const noexcept { return empty() ? nullptr : &buffer; }
          char*   get() const noexcept { return empty() ? nullptr : &buffer; }
    const char* c_str() const noexcept { return empty() ? nullptr : &buffer; }

    ptr_t<char>&  ptr() noexcept { return buffer; }

};

/*────────────────────────────────────────────────────────────────────────────*/

string_t operator+( const string_t& A, const string_t& B ){
    if( A.empty() ){ return B; } if( B.empty() ){ return A; }
    string_t C = string::buffer( A.size() + B.size() );
    memcpy( C.get()+ A.size(), B.get(), B.size() );
    memcpy( C.get(), A.get() , A.size() ); return C;
}

string_t operator^( const string_t& A, const string_t& B ){
    string_t C = string::buffer( A.size() );
    char *a=A.begin(), *b=B.begin(), *c=C.begin();
    while( c != C.end() ){ *c = *a ^ *b;
    ++a; ++b; ++c; } return C;
}

void operator^=( string_t& A, const string_t& B ){
    char *a=A.begin(), *b=B.begin(); /*-----------*/
    while( a != A.end() ){ *a = *a ^ *b; ++a; ++b; }
}

/*────────────────────────────────────────────────────────────────────────────*/

namespace string {

    string_t null(){ return buffer( 1, '\0' ); }

    string_t space(){ return buffer( 1, ' ' ); }

    /*─······································································─*/

    int to_int( const string_t& buffer ){
        int out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%d", &out ); return out;
    }

    bool to_bool( const string_t& buffer ){
        int out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%d", &out ); return out;
    }

    ldouble to_ldouble( const string_t& buffer ){
        ldouble out=0.0f; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%Lf", &out ); return out;
    }

    double to_double( const string_t& buffer ){
        double out=0.0f; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%lf", &out ); return out;
    }

    float to_float( const string_t& buffer ){
        float out=0.0f; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%f", &out ); return out;
    }

    char to_char( const string_t& buffer ){
        char out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%c", &out ); return out;
    }

    uint to_uint( const string_t& buffer ){
        uint out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%u", &out ); return out;
    }

    void* to_addr( const string_t& buffer ){
        void* out=nullptr; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%p", &out ); return out;
    }

    wchar to_wchar( const string_t& buffer ){
        wchar out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%lc", &out ); return out;
    }

    long to_long( const string_t& buffer ){
        long out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%ld", &out ); return out;
    }

    llong to_llong( const string_t& buffer ){
        llong out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%lld", &out ); return out;
    }


    ulong to_ulong( const string_t& buffer ){
        ulong out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%lu", &out ); return out;
    }

    ullong to_ullong( const string_t& buffer ){
        ullong out=0; if( buffer.empty() ){ return out; }
        sscanff( (char*) buffer, "%llu", &out ); return out;
    }

    /*─······································································─*/

    template< class... T >
    string_t format( const string_t& str, const T&... args ){
        char buffer[UNBFF_SIZE]; /*-----------------------*/
        snprintf( buffer, UNBFF_SIZE, (char*)str, args... );
        return buffer;
    }

    template< class... T >
    int parse( const string_t& data, const string_t& str, const T&... args ){
        return sscanff( (char*)data, (char*)str, args... );
    }

    /*─······································································─*/

    inline string_t to_string( char* num )/*------*/{ return num; }

    inline string_t to_string( const char* num )/**/{ return num; }

    inline string_t to_string( const string_t& num ){ return num; }

    string_t to_string( char num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%c", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( uint num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%u", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( int num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%d", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( long num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%ld", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( wchar num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%lc", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( ulong num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%lu", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( llong num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%lld", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( ullong num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%llu", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( double num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%lf", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( ldouble num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%Lf", num );
        return { buffer, (ulong)x };
    }

    template< class T > string_t to_string( T* num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%p", (void*)num );
        return { buffer, (ulong)x };
    }

    template< class T > string_t to_string( const ptr_t<T>& num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%p", (void*)&num );
        return { buffer, (ulong)x };
    }

    string_t to_string( float num ){
        char buffer[32]; auto x = snprintf( buffer, 32, "%lf", (double)num );
        return { buffer, (ulong)x };
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif