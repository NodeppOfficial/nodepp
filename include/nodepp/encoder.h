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

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_BASE8  "0123456789abcdef"
#define NODEPP_BASE58 "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
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

    inline uchar_64 atob( void* address, uchar_64 mask ){

        void* msk1   = &NODEPP_SHTDWN();
            
        uchar_64 raw = ( (uchar_64) address ) & (((uchar_64)-1)>>16);
        uchar_64 msk = ( (uchar_64) msk1    ) & (((uchar_64)-1)>>16);
        uchar_64 col =   (uchar_64) address   ^   (uchar_64) mask ;

        uchar_64 sum = ( col^(col>>16)^(col>>32)^(col>>48)) & 0xffff;
        return ( raw ^ msk ) | ( sum << 48 );

    }

    inline void* btoa( uchar_64 address, uchar_64 mask ){

        void* msk1   = &NODEPP_SHTDWN();

        uchar_64 msk = ( (uchar_64)  msk1  )& (((uchar_64)-1)>>16);
        void*    raw = (void*)((address^msk)& (((uchar_64)-1)>>16) );
        uchar_64 col = (uchar_64) raw       ^   (uchar_64) mask ;

        uchar_64 sum = ( col^(col>>16)^(col>>32)^(col>>48)) & 0xffff;
        uchar_64 out = ( address >>48) /*----------------*/ & 0xffff;
        return out==sum ? raw : nullptr ; 

    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace hash {

    inline ulong get( const string_t& key, int tableSize ) {
        ulong hash = 0x1505; forEach( x, key ){
              hash = ((hash << 5) + hash) + x;
        }     return hash % tableSize;
    }

    inline ulong get( int key, int tableSize ){ return key % tableSize; }

    inline ulong get( const string_t& key )   { return get( key, NODEPP_HASH_TABLE_SIZE ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace XOR {

    inline string_t atob( const string_t& data, const string_t& key ){
        auto  tmp= data.copy();
        ulong pos= 0; forEach( x, tmp ) {
            x = x^ key[pos]; ++pos;
            pos %= key.size();
        }   return tmp;
    }

    inline string_t btoa( const string_t& data, const string_t& key ){
        auto  tmp= data.copy();
        ulong pos= 0; forEach( x, tmp ) {
            x = x^ key[pos]; ++pos;
            pos %= key.size();
        }   return tmp;
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace bytes {

    template< class T >
    ptr_t<uchar> atob( T num ){ ptr_t<uchar> out( sizeof( num ), 0 );
     auto tmp = typename type::make_unsigned<T>::type( num );
        for( ulong y=0; y<out.size(); ++y ){
             out[y] = tmp >> ( 8*(out.size()-y-1) );
        }
    return out; }

    template< class T >
    T btoa( const ptr_t<uchar>& num ){
    auto out = typename type::make_unsigned<T>::type( 0x00 );
        for( ulong y=0; y<num.size(); ++y ){
             out = ( out << 8 ) | num[y];
        }    
    return out; }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace bin {

    template< class T >
    ptr_t<bool> atob( T num ){ ptr_t<bool> out ( sizeof ( num ) * 8, 0 );
    auto tmp = typename type::make_unsigned<T>::type( num );
        for( auto x= out.size(); x-- >0; ){
             out[x]= tmp & 0x01; tmp >>= 1;
        }    return out;
    }

    template< class T >
    T btoa( const ptr_t<bool>& num ){ 
    auto out = typename type::make_unsigned<T>::type( 0x00 );
        if ( num.empty () ){ return out; }
        for( auto& x: num ){ out = ( out << 0x01 ) | ( x & 0x01 ); }
    return out; }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace hex {

    template< class T, class = typename type::enable_if<type::is_integral<T>::value,T>::type >
    string_t atob( T num ){ auto tmp = typename type::make_unsigned<T>::type( num );
    string_t out ; do {
         out.unshift( NODEPP_BASE8[ tmp & 0x0F ] ); tmp >>= 4;
    } while( tmp != 0 ); if( out.size() % 2 != 0 ){
         out.unshift( '0' );
    } return out; }

    template< class T, class = typename type::enable_if<type::is_integral<T>::value,T>::type >
    T btoa( const string_t& num ){ if ( num.empty() ){ return 0; }
        auto out = typename type::make_unsigned<T>::type (0x00);
        for ( auto c: num ){ out  = out << 4;
        if  ( c >= '0' && c <= '9' ){ out |= c - '0'     ; }
        elif( c >= 'a' && c <= 'f' ){ out |= c - 'a' + 10; }
        elif( c >= 'A' && c <= 'F' ){ out |= c - 'A' + 10; }
        else{ return 0; }} return out;
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace hex {

    inline string_t atob( const ptr_t<uchar>& inp ){
        if ( inp.empty() ) { return nullptr; }
        queue_t<char> out; for( auto x : inp ){
            for( auto y: atob(x) ){ out.push( y ); }
        }   out.push('\0'); return string_t( out.data() );
    }

    inline ptr_t<uchar> btoa( string_t x ){
        if ( x.empty() ){ return nullptr; }
        ulong size = x.size()/2 + ( x.size()%2 != 0?1:0 );
        ptr_t<uchar> out(size,'\0'); for( auto &y : out ){
            y = btoa<uchar>( x.splice(0,2) );
        }   return out;
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace base16 {

    inline string_t btoa( const string_t& inp ){
        if( inp.empty() ){ return nullptr; }
        ptr_t<uchar> buff = hex::btoa(inp);
        string_t raw ( buff.size() + 1 );
        memcpy( raw.get(), &buff, buff.size() );
        return raw;
    }

    inline string_t atob( const string_t& inp ){
        if( inp.empty() ){ return nullptr; }
        auto raw = ptr_t<uchar>( inp.size() );
        memcpy( &raw, inp.get(), inp.size() );
        return hex::atob( raw );
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace base64 {

    inline string_t atob( const string_t &inp ) {

        queue_t<char> out; int pos1 = 0, pos2 = -6;

        for  ( uchar c: inp ) {
               pos1= ( pos1 << 8 ) + c; pos2 += 8;
        while( pos2 >= 0 ) {
               out.push(NODEPP_BASE64[(pos1>>pos2)&0x3F]);
               pos2 -= 6;
        }}

        if( pos2>-6 ){ out.push(NODEPP_BASE64[((pos1<<8)>>(pos2+8))&0x3F]); }
        while( out.size()%4 ){ out.push('='); } out.push('\0'); 
        
        return string_t( out.data() );
    }

    inline string_t btoa( const string_t &in ) {

        queue_t<char> out; int pos1=0, pos2=-8; 
        int T[256]; type::fill( T, T+256, -1 );

        for( int i=0; i<64; ++i ){ T[NODEPP_BASE64[i]]=i; }
        for( uchar c: in ) { if ( T[c]==-1 ){ break; }
             pos1 = ( pos1 << 6 )+T[c] ; pos2 += 6;
        if ( pos2 >= 0 ) {
             out.push(char((pos1>>pos2)&0xFF));
             pos2 -= 8;
        }}

        out.push('\0'); return string_t( out.data() );
    }

}}}

namespace nodepp { namespace encoder { namespace base58 {

    string_t atob( string_t message ){
        
        if( message.empty() ){ return nullptr; } 
        
        ptr_t  <uchar> nmb( message.size() );
        queue_t<char>  out, zrs;

        memcpy( nmb.get(), message.get(), nmb.size() );
        for( auto &x: nmb ){ if( x != 0x00 ){ break; }
            zrs.push('1'); nmb.slice( 1, nmb.size() );
        }
        
        while( !nmb.empty() ){ 
            
            uchar_32 borrow = 0;

            for( auto &x: nmb ){
                uchar_32 current = ( borrow<<8 ) | x;
                x      = current / 58;
                borrow = current % 58;
            }

            while(!nmb.empty() && nmb[0]==0x00 )
                 { nmb.slice( 1, nmb.size() ); }
            out.unshift( NODEPP_BASE58[borrow] );

        }
        
        out.insert( out.first(), zrs.data() ); 
        out.push( '\0' ); return out.data();

    }

    string_t btoa( string_t message ){

        if ( message.empty() ){ return nullptr; } 
        int T[256]; type::fill( T, T+256, -1 );
        for( int i=0; i<58; i++ ){ T[ (uchar) NODEPP_BASE58[i] ] = i; }
        
        array_t<uchar> nmb;
        queue_t<char>  out, zrs;

        for( auto &x: nmb ){ if( x != NODEPP_BASE58[0] ){ break; }
            zrs.push( 0x00 ); nmb.shift();
        }
        
        for( ulong x=zrs.size(); x<message.size(); x++ ){
        if ( T[message[x]] < 0 ){ continue; } 

            uchar_32 borrow = T[message[x]];

            for( auto &x: nmb ){
                 uchar_32 current = ( x * 58 ) + borrow;
                 x      = current & 0xFF;
                 borrow = current >> 8;
            }

            while( borrow > 0 ){ nmb.push( borrow & 0xFF ); borrow >>= 8; }

        }
        
        out.insert( nullptr, zrs.data() );
        out.insert( nullptr, nmb.reverse().ptr() ); 
        out.push( '\0' ); return out.data();
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace utf8 {
    inline ptr_t<uchar_16> to_utf16( const ptr_t<uchar_8>& inp ){ return utf::utf8_to_utf16( inp ); }
    inline ptr_t<uchar_32> to_utf32( const ptr_t<uchar_8>& inp ){ return utf::utf8_to_utf32( inp ); }
}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace utf16 {
    inline ptr_t<uchar_8>  to_utf8 ( const ptr_t<uchar_16>& inp ){ return utf::utf16_to_utf8 ( inp ); }
    inline ptr_t<uchar_32> to_utf32( const ptr_t<uchar_16>& inp ){ return utf::utf16_to_utf32( inp ); }
}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace encoder { namespace utf32 {
    inline ptr_t<uchar_8>  to_utf8 ( const ptr_t<uchar_32>& inp ){ return utf::utf32_to_utf8 ( inp ); }
    inline ptr_t<uchar_16> to_utf16( const ptr_t<uchar_32>& inp ){ return utf::utf32_to_utf16( inp ); }
}}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef NODEPP_BASE58
#undef NODEPP_BASE64
#undef NODEPP_BASE8
#endif

/*────────────────────────────────────────────────────────────────────────────*/