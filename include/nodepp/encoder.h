/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ENCODER
#define NODEPP_ENCODER
#define NODEPP_BASE8  "0123456789abcdef"
#define NODEPP_BASE64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

/*────────────────────────────────────────────────────────────────────────────*/

#include "string.h"
#include "utf.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace key {

    inline string_t generate( const string_t& alph, int x=32 ){ ulong idx=0;
        string_t data ( (ulong)x, '\0' ); for( auto &x: data ){
        x = alph[rand()%(alph.size())]; ++idx; } return data;
    }

    inline string_t generate( int x=32 ) { return generate( NODEPP_BASE64, x ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace ofuscator { 

    inline uchar_64 atob( void* address, void* sign_ptr ){

        void* msk1   = &NODEPP_SHTDWN();
        void* msk2   = sign_ptr;
            
        uchar_64 raw = ( (uchar_64) address ) & (((uchar_64)-1)>>16);
        uchar_64 msk = ( (uchar_64) msk1    ) & (((uchar_64)-1)>>16);
        uchar_64 col =   (uchar_64) address   ^   (uchar_64) msk2 ;

        uchar_64 sum = ( col^(col>>16)^(col>>32)^(col>>48)) & 0xffff;
        return ( raw ^ msk ) | ( sum << 48 );

    }

    inline void* btoa( uchar_64 address, void* sign_ptr ){

        void* msk1   = &NODEPP_SHTDWN();
        void* msk2   = sign_ptr;

        uchar_64 msk = ( (uchar_64)  msk1  )& (((uchar_64)-1)>>16);
        void*    raw = (void*)((address^msk)& (((uchar_64)-1)>>16) );
        uchar_64 col = (uchar_64) raw       ^   (uchar_64) msk2 ;

        uchar_64 sum = ( col^(col>>16)^(col>>32)^(col>>48)) & 0xffff;
        uchar_64 out = ( address >>48) /*----------------*/ & 0xffff;
        return out==sum ? raw : nullptr ; 

    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace hash {

    inline ulong get( const string_t& key, int tableSize ) {
        ulong hash = 5381; forEach( x, key ) {
              hash = ((hash << 5) + hash) + x;
        }     return hash % tableSize;
    }

    inline ulong get( int key, int tableSize ) { return key % tableSize; }

    inline ulong get( const string_t& key )    { return get( key, NODEPP_HASH_TABLE_SIZE ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace XOR {

    inline string_t get( string_t data, const string_t& key ){
        auto  tmp= data.copy();
        ulong pos= 0; forEach( x, tmp ) {
            x = x^ key[pos]; ++pos;
            pos %= key.size();
        }   return tmp;
    }

    inline string_t set( string_t data, const string_t& key ){
        auto  tmp= data.copy();
        ulong pos= 0; forEach( x, tmp ) {
            x = x^ key[pos]; ++pos;
            pos %= key.size();
        }   return tmp;
    }

    /*─······································································─*/

    template< class... T >
    string_t atob( T... args ) { return get( args... ); }

    template< class... T >
    string_t btoa( T... args ) { return set( args... ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace bytes {

    template< class T >
    ptr_t<uchar> get( T num ){
        ptr_t<uchar> out ( sizeof(num), 0 );
        for( ulong y=0; y<out.size(); ++y ){
             out[y] = num >> ( 8*(out.size()-y-1) );
        }    return out;
    }

    template< class T >
    T set( const ptr_t<uchar>& num ){ T out;
      for( ulong y=0; y<num.size(); ++y ){
           out = out << 8 | num[y];
      }    return out;
    }

    /*─······································································─*/

    template< class T >
    ptr_t<uchar> atob( T num ) { return get( num ); }

    template< class T >
    T btoa( const ptr_t<uchar>& num ) { return set<T>( num ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace bin {

    template< class T >
    ptr_t<bool> get( T num ){
    ptr_t<bool> out ( sizeof(num) * 8, 0 );
        for ( auto x=sizeof(num)*8; x--; ){
              out[x] = num & 1 ; num >>= 1;
        }     return out;
    }

    template< class T >
    T set( const ptr_t<bool>& num ){ T out = 0;
        if  ( num.empty() ){ return out; }
        for ( auto& x : num ){
              out = out << 1 | ( x & 1 );
        }     return out;
    }

    /*─······································································─*/

    template< class T >
    ptr_t<bool> atob( T num ) { return get( num ); }

    template< class T >
    T btoa( const ptr_t<bool>& num ) { return set<T>( num ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace hex {

    template< class T, class = typename type::enable_if<type::is_integral<T>::value,T>::type >
    string_t get( T num ){ string_t out; do {
             out.unshift( NODEPP_BASE8[num&(T)(0xf)] ); num >>= 4;
        } while( num != 0 ); if( out.size()%2!=0 ){
             out.unshift( '0' );
        } return out;
    }

    template< class T, class = typename type::enable_if<type::is_integral<T>::value,T>::type >
    T set( string_t num ){ if ( num.empty() ){ return 0; }
        T out = 0; for ( auto c: num ){    out  = out<<4;
            if   ( c >= '0' && c <= '9' ){ out |= c - '0'     ; }
            elif ( c >= 'a' && c <= 'f' ){ out |= c - 'a' + 10; }
            elif ( c >= 'A' && c <= 'F' ){ out |= c - 'A' + 10; }
            else { return 0; }
        }   return out;
    }

    /*─······································································─*/

    template< class T, class = typename type::enable_if<type::is_integral<T>::value,T>::type >
    string_t atob( T num ) { return get( num ); }

    template< class T, class = typename type::enable_if<type::is_integral<T>::value,T>::type >
    T btoa( string_t num ) { return set<T>( num ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace hex {

    inline string_t get( const ptr_t<uchar>& inp ){
        if ( inp.empty() ){ return nullptr; }
        queue_t<char> out; for( auto x : inp ){
            for( auto y: get(x) ){ out.push( y ); }
        }   out.push('\0'); return string_t( out.data() );
    }

    inline ptr_t<uchar> set( string_t x ){
        if ( x.empty() ){ return nullptr; }
        ulong size = x.size()/2 + ( x.size()%2 != 0?1:0 );
        ptr_t<uchar> out(size,'\0'); for( auto &y : out ){
            y = set<uchar>( x.splice(0,2) );
        }   return out;
    }

    /*─······································································─*/

    inline ptr_t<uchar> btoa( string_t inp ) { return set( inp ); }

    inline string_t atob( const ptr_t<uchar>& inp ) { return get( inp ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace buffer {

    inline string_t hex2buff( const string_t& inp ){
        if( inp.empty() ){ return nullptr; }
        ptr_t<uchar> buff = hex::set(inp);
        string_t raw ( buff.size() + 1 );
        memcpy( raw.get(), &buff, buff.size() );
        return raw;
    }

    inline string_t buff2hex( const string_t& inp ){
        if( inp.empty() ){ return nullptr; }
        auto raw = ptr_t<uchar>( inp.size() );
        memcpy( &raw, inp.get(), inp.size() );
        return hex::get( raw );
    }

    /*─······································································─*/

    inline string_t atob( const string_t& inp ) { return buff2hex( inp ); }

    inline string_t btoa( const string_t& inp ) { return hex2buff( inp ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace base16 {

    inline string_t atob( const string_t& inp ) { return buffer::buff2hex( inp ); }

    inline string_t btoa( const string_t& inp ) { return buffer::hex2buff( inp ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace base64 {

    inline string_t get( const string_t &in ) {

        queue_t<char> out; int pos1 = 0, pos2 = -6;

        for ( uchar c: in ) {
            pos1= ( pos1 << 8 ) + c; pos2 += 8;
            while ( pos2 >= 0 ) {
                out.push(NODEPP_BASE64[(pos1>>pos2)&0x3F]);
                pos2 -= 6;
            }
        }

        if( pos2>-6 ){ out.push(NODEPP_BASE64[((pos1<<8)>>(pos2+8))&0x3F]); }
        while( out.size()%4 ){ out.push('='); } out.push('\0'); 
        
        return string_t( out.data() );
    }

    inline string_t set( const string_t &in ) {

        queue_t<char> out; int pos1=0, pos2=-8; ptr_t<int> T( 256, -1 );

        for ( int i=0; i<64; ++i ) T[NODEPP_BASE64[i]] = i;
        for ( uchar c: in ) { if ( T[c]==-1 ) break;
            pos1 = ( pos1 << 6 ) + T[c]; pos2 += 6;
            if (pos2 >= 0) {
                out.push(char((pos1>>pos2)&0xFF));
                pos2 -= 8;
            }
        }

        out.push('\0'); return string_t( out.data() );
    }

    /*─······································································─*/

    inline string_t btoa( const string_t &in ) { return set( in ); }

    inline string_t atob( const string_t &in ) { return get( in ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace ctz { 

    inline int count( uchar_8 value ) noexcept {
        if ( value == 0 ) { return 8; } int count = 0;
        if ( ( value & 0x0FU ) == 0x00U ) { count += 4; value >>= 4; }
        if ( ( value & 0x03U ) == 0x00U ) { count += 2; value >>= 2; }
        return count + ( 1 - ( value & 0x01U ) );
    }

    inline int count( uchar_16 value ) noexcept {
        if ( value == 0 ) { return 16; } int count = 0;
        if ( ( value & 0x00FFU ) == 0x0000U ) { count += 8; value >>= 8; }
        if ( ( value & 0x000FU ) == 0x0000U ) { count += 4; value >>= 4; }
        if ( ( value & 0x0003U ) == 0x0000U ) { count += 2; value >>= 2; }
        return count + ( 1 - ( value & 0x01U ) );
    }

    inline int count( uchar_32 value ) noexcept {
        if ( value == 0 ) { return 32; } int count = 0;
        if ( ( value & 0x0000FFFFUL ) == 0x00000000UL ) { count += 16; value >>= 16; }
        if ( ( value & 0x000000FFUL ) == 0x00000000UL ) { count += 8;  value >>= 8;  }
        if ( ( value & 0x0000000FUL ) == 0x00000000UL ) { count += 4;  value >>= 4;  }
        if ( ( value & 0x00000003UL ) == 0x00000000UL ) { count += 2;  value >>= 2;  }
        return count + ( 1 - ( value & 0x01UL ) );
    }

    inline int count( uchar_64 value ) noexcept {
        if ( value == 0 ) { return 64; } int count = 0;
        if ( ( value & 0xFFFFFFFFULL ) == 0x0000000000000000ULL ) { count += 32; value >>= 32; }
        if ( ( value & 0x0000FFFFULL ) == 0x0000000000000000ULL ) { count += 16; value >>= 16; }
        if ( ( value & 0x000000FFULL ) == 0x0000000000000000ULL ) { count += 8;  value >>= 8;  }
        if ( ( value & 0x0000000FULL ) == 0x0000000000000000ULL ) { count += 4;  value >>= 4;  }
        if ( ( value & 0x00000003ULL ) == 0x0000000000000000ULL ) { count += 2;  value >>= 2;  }
        return count + ( 1 - ( value & 0x01ULL ) );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace utf8 {
    inline ptr_t<uchar_16> to_utf16( ptr_t<uchar_8> inp ){ return utf::utf8_to_utf16( inp ); }
    inline ptr_t<uchar_32> to_utf32( ptr_t<uchar_8> inp ){ return utf::utf8_to_utf32( inp ); }
}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace utf16 {
    inline ptr_t<uchar_8>  to_utf8 ( ptr_t<uchar_16> inp ){ return utf::utf16_to_utf8 ( inp ); }
    inline ptr_t<uchar_32> to_utf32( ptr_t<uchar_16> inp ){ return utf::utf16_to_utf32( inp ); }
}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace utf32 {
    inline ptr_t<uchar_8>  to_utf8 ( ptr_t<uchar_32> inp ){ return utf::utf32_to_utf8 ( inp ); }
    inline ptr_t<uchar_16> to_utf16( ptr_t<uchar_32> inp ){ return utf::utf32_to_utf16( inp ); }
}}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef NODEPP_BASE64
#undef NODEPP_BASE8
#endif

/*────────────────────────────────────────────────────────────────────────────*/