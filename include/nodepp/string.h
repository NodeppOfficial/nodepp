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

public:

    string_t() noexcept { buffer = nullptr; }

    string_t( const char* argc ) noexcept {
        if( argc == nullptr ){
            buffer = nullptr; return;
        }   buffer = string::buffer( argc, strlen(argc) );
    }

    string_t( const ulong& n, const char& c ) noexcept {
        if( n == 0 ){
            buffer = nullptr; return;
        }   buffer = string::buffer( n, c );
    }

    string_t( const char* argc, const ulong& n ) noexcept {
        if( argc == nullptr || n == 0 ){
            buffer = nullptr; return;
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

    string_t operator+=( const string_t& oth ){
        if( oth.empty() ){ return *this; } string_t ths = copy(); ulong n = 0;
        buffer = string::buffer( this->size() + oth.size() );
        for( auto x : ths ){ (*this)[n] = x; n++; }
        for( auto x : oth ){ (*this)[n] = x; n++; } return *this;
    }

    /*─······································································─*/

    bool operator> ( const string_t& oth ) const noexcept { return compare( oth ) == 1; }
    bool operator>=( const string_t& oth ) const noexcept { return compare( oth ) >= 0; }
    bool operator<=( const string_t& oth ) const noexcept { return compare( oth ) <= 0; }
    bool operator< ( const string_t& oth ) const noexcept { return compare( oth ) ==-1; }
    bool operator==( const string_t& oth ) const noexcept { return compare( oth ) == 0; }
    bool operator!=( const string_t& oth ) const noexcept { return compare( oth ) != 0; }

    char& operator[]( ulong n ) const noexcept { return buffer[n]; }

    /*─······································································─*/

    long index_of( function_t<bool,char> func ) const noexcept { long i=0;
        for( auto& x : *this ){ if( func(x) ) return i; i++; } return -1;
    }

    ulong count( function_t<bool,char> func ) const noexcept { ulong n=0;
        for( auto& x : *this ){ if( func(x) ) n++; } return n;
    }

    /*─······································································─*/

    char reduce( function_t<char,char,char> func ) const noexcept { char act = (*this)[0];
        for( auto x=this->begin() + 1; x != this->end(); x++ )
           { act = func( act, *x ); } return act;
    }

    bool some( function_t<bool,char> func ) const noexcept {
        for( auto& x : *this ){ if( func(x)==1 ) return 1; } return 0;
    }

    bool none( function_t<bool,char> func ) const noexcept {
        for( auto& x : *this ){ if( func(x)==1 ) return 0; } return 1;
    }

    bool every( function_t<bool,char> func ) const noexcept {
        for( auto& x : *this ){ if(!func(x)==0 ) return 0; } return 1;
    }

    void map( function_t<void,char&> func ) const noexcept {
        for( auto& x : *this ) func(x);
    }

    /*─······································································─*/

    ptr_t<int> find( const string_t& data, ulong offset=0 ) const noexcept {
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

    ptr_t<int> find( const char& data, ulong offset=0 ) const noexcept {
        return find( string_t( 1UL, data ), offset );
    }

    /*─······································································─*/

    int compare( const string_t& oth ) const noexcept {
        if( size() < oth.size() ){ return -1; }
        if( size() > oth.size() ){ return  1; }
        ulong n = size(); while( n-->0 ){
            if( (*this)[n] < oth[n] ){ return -1; }
            if( (*this)[n] > oth[n] ){ return  1; }
        }   return 0;
    }

    /*─······································································─*/

    string_t remove( function_t<bool,char> func ) noexcept {
        ulong n=size(); while( n-->0 ){ if( func((*this)[n]) ) erase(n); } return (*this);
    }

    string_t reverse() const noexcept { auto n_buffer = copy();
        ulong n=size(); for( auto& x : *this ){ n--; n_buffer[n]=x; } return n_buffer;
    }

    string_t replace( function_t<bool,char> func, char targ ) const noexcept {
        for( auto& x : *this ){ if(func(x)) x=targ; } return (*this);
    }

    string_t copy() const noexcept { return buffer.copy(); }

    /*─······································································─*/

    void fill( const char& argc ) const noexcept { buffer.fill(argc); }

    template< class... T >
    void resize( T... args ) noexcept { buffer.resize(args...); }

    /*─······································································─*/

    string_t sort( function_t<bool,char,char> func ) const noexcept {
        queue_t<char> n_buffer;

        for( ulong i=0; i<size(); i++ ){
            auto x = buffer[i]; auto n = n_buffer.first();
            while( n!=nullptr ){
               if( !func( x, n->data ) )
                 { n = n->next; continue; } break;
            }      n_buffer.insert( n, x );
        }          n_buffer.push('\0');

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
        if( empty() ){ buffer = string::buffer( 1 ); buffer[0] = value; }
        else { ulong n=size() + 1; auto n_buffer = string::buffer(n);
            for( ulong x=0,i=0; x<n; x++ ){
                if( x == index ){ n_buffer[x] = value; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N , char* value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( value, N ); }
        else { ulong n=size() + N; auto n_buffer = string::buffer(n);
            for( ulong x=0,i=0,p=0; x<n; x++ ){
                if( x>=index && x<index+N ){ n_buffer[x] = value[p++]; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    void insert( ulong index, ulong N , const char& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( N, value ); }
        else { ulong n=size() + N; auto n_buffer = string::buffer(n);
            for( ulong x=0,i=0; x<n; x++ ){
                if( x>=index && x<index+N ){ n_buffer[x] = value; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    void insert( ulong index, const string_t& value ) noexcept {
	    index = clamp( index, 0UL, size() );
        if( empty() ){ buffer = string::buffer( value.size() );
            for( ulong i=0; i<value.size(); i++ ){ buffer[i] = value[i]; }
        } else { ulong n=size() + value.size(); auto n_buffer = string::buffer(n);
            for( ulong x=0,i=0,p=0; x<n; x++ ){
                if( x>=index && x<index+value.size() ){ n_buffer[x] = value[p++]; }
                else { n_buffer[x] = buffer[i++]; }
            }   buffer = n_buffer;
        }
    }

    /*─······································································─*/

    void erase( ulong index ) noexcept {
	    auto r = get_slice_range( index, size() );
         if( r == nullptr ){ return; } else {
            auto n_buffer = string::buffer( size() - 1 );
            for( ulong i=0, j=0; i<size(); i++ ){ if( i != r[0] )
               { n_buffer[j] = buffer[i]; j++; }
            }    buffer = n_buffer;
        }
    }

    void erase( ulong start, ulong end  ) noexcept {
	    auto r = get_slice_range( start, end );
         if( r == nullptr ){ return; } else {
            auto n_buffer = string::buffer( size() - (r[1]-r[0]) - 1 );
            for( ulong i=0, j=0; i<=last(); i++ ){ if( i<r[0] || i>r[1] )
               { n_buffer[j] = buffer[i]; j++; }
            }    buffer = n_buffer;
        }
    }

    /*─······································································─*/

    string_t slice( long start ) const noexcept {

        auto r = get_slice_range( start, size() );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = string_t( buffer.data()+r[0], r[2] );
        return n_buffer;
    }

    /*─······································································─*/

    string_t slice( long start, long end ) const noexcept {

        auto r = get_slice_range( start, end );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = string_t( buffer.data()+r[0], r[2] );
        return n_buffer;
    }

    /*─······································································─*/

    string_t splice( long start, ulong end ) noexcept {

        auto r = get_splice_range( start, end );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = string_t( buffer.data()+r[0], r[2] );
        erase( r[0], r[0]+end ); return n_buffer;
    }

    template< class V >
    string_t splice( long start, ulong end, const V& value ) noexcept {

        auto r = get_splice_range( start, end );
         if( r == nullptr ){ return nullptr; }

        auto n_buffer = string_t( buffer.data()+r[0], r[2] );
        erase( r[0], r[0]+end ); insert( r[0], value ); return n_buffer;
    }

    /*─······································································─*/

    string_t to_capital_case() const noexcept {
        if ( empty() ){ return nullptr; } bool b=1; ptr_t<char> res (size()+1,0);
        for( ulong x=0; x<res.size(); x++ ){ auto y = buffer[x];
            if( string::is_alpha(y) && b==1 ){ res[x] = string::to_upper(y); b=0; continue; }
            if(!string::is_alpha(y) ){ b =1;}  res[x] = string::to_lower(y);
        }   return res;
    }

    string_t to_lower_case() const noexcept {
        if ( empty() ){ return nullptr; } ptr_t<char> res (size()+1,0);
        for( ulong x=0; x<res.size(); x++ ){
             res[x] = string::to_lower( buffer[x] );
        }    return res;
    }

    string_t to_upper_case() const noexcept {
        if ( empty() ){ return nullptr; } ptr_t<char> res (size()+1,0);
        for( ulong x=0; x<res.size(); x++ ){
             res[x] = string::to_upper( buffer[x] );
        }    return res;
    }

    string_t to_slugify() const noexcept { ulong z=0;
        if ( empty() ){ return nullptr; } ptr_t<char> res (size()+1,0);
        for( ulong x=0; x<res.size(); x++ ){ auto y = buffer[x];
              if ( !string::is_alnum(y) ) { continue; }
            else { res[z] = string::to_lower(y); z++; }
        }   return { &res, z };
    }

    /*─······································································─*/

    explicit operator char* (void) const noexcept { return empty() ? (char*)"" : &buffer; }
          char*  data() const noexcept { return empty() ? (char*) "" : &buffer; }
          char*   get() const noexcept { return empty() ? (char*) "" : &buffer; }
    const char* c_str() const noexcept { return empty() ?         "" : &buffer; }
    explicit operator bool(void) const noexcept { return empty(); }
    ptr_t<char>&  ptr() noexcept { return buffer; }

};

/*────────────────────────────────────────────────────────────────────────────*/

string_t operator+( const string_t& A, const string_t& B ){
    string_t C = string::buffer( A.size() + B.size() );
    memcpy( C.get()+ A.size(), B.get(), B.size() );
    memcpy( C.get(), A.get() , A.size() ); return C;
}

string_t operator^( const string_t& A, const string_t& B ){
    string_t C = string::buffer( A.size() );
    for( ulong x=0; x<C.size(); x++ )
       { C[x] = A[x] ^ B[x%B.size()]; }
    return C;
}

void operator^=( string_t& A, const string_t& B ){
    for( ulong x=0; x<A.size(); x++ )
       { A[x] = A[x] ^ B[x%B.size()]; }
}

/*────────────────────────────────────────────────────────────────────────────*/

namespace string {

    int to_int( const string_t& buffer ){
        int out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%d", &out ); return out;
    }

    bool to_bool( const string_t& buffer ){
        int out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%d", &out ); return out;
    }

    ldouble to_ldouble( const string_t& buffer ){
        ldouble out=0.0f; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%Lf", &out ); return out;
    }

    double to_double( const string_t& buffer ){
        double out=0.0f; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%lf", &out ); return out;
    }

    float to_float( const string_t& buffer ){
        float out=0.0f; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%f", &out ); return out;
    }

    char to_char( const string_t& buffer ){
        char out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%c", &out ); return out;
    }

    uint to_uint( const string_t& buffer ){
        uint out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%u", &out ); return out;
    }

    void* to_addr( const string_t& buffer ){
        void* out=nullptr; if( buffer.empty() ){ return out; }
        sprintf( (char*) buffer, "%p", out ); return out;
    }

    wchar to_wchar( const string_t& buffer ){
        wchar out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%lc", &out ); return out;
    }

    long to_long( const string_t& buffer ){
        long out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%ld", &out ); return out;
    }

    llong to_llong( const string_t& buffer ){
        llong out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%lld", &out ); return out;
    }


    ulong to_ulong( const string_t& buffer ){
        ulong out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%lu", &out ); return out;
    }

    ullong to_ullong( const string_t& buffer ){
        ullong out=0; if( buffer.empty() ){ return out; }
        sscanf( (char*) buffer, "%llu", &out ); return out;
    }

    /*─······································································─*/

    template< class... T >
    string_t format( const string_t& str, const T&... args ){
        char   buffer[UNBFF_SIZE]; sprintf( buffer, (char*)str, args... );
        return buffer;
    }

    template< class... T >
    int parse( const string_t& data, const string_t& str, const T&... args ){
        return sscanf( (char*)data, (char*)str, args... );
    }

    /*─······································································─*/

    inline string_t to_string( char* num ){ return num; }

    inline string_t to_string( const char* num ){ return num; }

    inline string_t to_string( const string_t& num ){ return num; }

    string_t to_string( char num ){
        char buffer[32]; auto x = sprintf( buffer, "%c", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( uint num ){
        char buffer[32]; auto x = sprintf( buffer, "%u", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( int num ){
        char buffer[32]; auto x = sprintf( buffer, "%d", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( long num ){
        char buffer[32]; auto x = sprintf( buffer, "%ld", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( wchar num ){
        char buffer[32]; auto x = sprintf( buffer, "%lc", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( ulong num ){
        char buffer[32]; auto x = sprintf( buffer, "%lu", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( llong num ){
        char buffer[32]; auto x = sprintf( buffer, "%lld", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( ullong num ){
        char buffer[32]; auto x = sprintf( buffer, "%llu", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( double num ){
        char buffer[32]; auto x = sprintf( buffer, "%lf", num );
        return { buffer, (ulong)x };
    }

    string_t to_string( ldouble num ){
        char buffer[32]; auto x = sprintf( buffer, "%Lf", num );
        return { buffer, (ulong)x };
    }

    template< class T > string_t to_string( T* num ){
        char buffer[32]; auto x = sprintf( buffer, "%p", (void*)num );
        return { buffer, (ulong)x };
    }

    template< class T > string_t to_string( const ptr_t<T>& num ){
        char buffer[32]; auto x = sprintf( buffer, "%p", (void*)&num );
        return { buffer, (ulong)x };
    }

    string_t to_string( float num ){
        char buffer[32]; auto x = sprintf( buffer, "%lf", (double)num );
        return { buffer, (ulong)x };
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif
