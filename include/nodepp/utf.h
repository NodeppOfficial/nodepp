/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_UTF_CONVERTER
#define NODEPP_UTF_CONVERTER

namespace nodepp { namespace utf {

/*────────────────────────────────────────────────────────────────────────────*/

inline ptr_t<uchar_32> utf8_to_utf32( const ptr_t<uchar_8>& utf8 ) {
    queue_t<uchar_32> utf32; ulong i=0; while ( i<utf8.size() ) {
        uchar_32 codepoint = 0; uchar_8 byte = utf8[i];

        if (byte < 0x80) {
            codepoint = byte; i += 1;
        } elif ((byte & 0xE0) == 0xC0) {
            if (i + 1 >= utf8.size()){ return nullptr; }
            codepoint = static_cast<uchar_32>(byte & 0x1F) << 6 | (utf8[i + 1] & 0x3F);
            i += 2;
        } elif ((byte & 0xF0) == 0xE0) {
            if (i + 2 >= utf8.size()){ return nullptr; }
            codepoint = static_cast<uchar_32>(byte & 0x0F) << 12 | (utf8[i + 1] & 0x3F) << 6 | (utf8[i + 2] & 0x3F);
            i += 3;
        } elif ((byte & 0xF8) == 0xF0) {
            if (i + 3 >= utf8.size()){ return nullptr; }
            codepoint = static_cast<uchar_32>(byte & 0x07) << 18 | (utf8[i + 1] & 0x3F) << 12 | (utf8[i + 2] & 0x3F) << 6 | (utf8[i + 3] & 0x3F);
            i += 4;
        } else { return nullptr; }

        utf32.push( codepoint );
    }   return utf32.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

inline ptr_t<uchar_8> utf32_to_utf8( const ptr_t<uchar_32>& utf32 ) {
    queue_t<uchar_8> utf8; for ( uchar_32 codepoint : utf32 ) {
        if (codepoint <= 0x7F) {
            utf8.push( type::cast<uchar_8>(codepoint) );
        } elif (codepoint <= 0x7FF) {
            utf8.push( type::cast<uchar_8>((codepoint >>   6) | 0xC0) );
            utf8.push( type::cast<uchar_8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0xFFFF) {
            utf8.push( type::cast<uchar_8>((codepoint  >> 12) | 0xE0) );
            utf8.push( type::cast<uchar_8>(((codepoint >>  6) & 0x3F) | 0x80) );
            utf8.push( type::cast<uchar_8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0x10FFFF) {
            utf8.push( type::cast<uchar_8>((codepoint >> 18)  | 0xF0) );
            utf8.push( type::cast<uchar_8>(((codepoint >> 12) & 0x3F) | 0x80) );
            utf8.push( type::cast<uchar_8>(((codepoint >> 6)  & 0x3F) | 0x80) );
            utf8.push( type::cast<uchar_8>((codepoint & 0x3F) | 0x80) );
        } else { return nullptr; }
    }   return utf8.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

inline ptr_t<uchar_16> utf8_to_utf16( const ptr_t<uchar_8>& utf8 ) {
    queue_t<uchar_16> utf16; ulong i = 0; while ( i<utf8.size() ) {
        uchar_32 codepoint = 0; uchar_8 byte = utf8[i];

        if (byte < 0x80) {
            codepoint = byte; i += 1;
        } elif ((byte & 0xE0) == 0xC0) {
            if (i+1 >= utf8.size()){ return nullptr; }
            codepoint = static_cast<uchar_32>(byte & 0x1F) << 6 | (utf8[i + 1] & 0x3F); 
            i += 2;
        } elif ((byte & 0xF0) == 0xE0) {
            if (i+2 >= utf8.size()){ return nullptr; }
            codepoint = static_cast<uchar_32>(byte & 0x0F) << 12 | (utf8[i + 1] & 0x3F) << 6 | (utf8[i + 2] & 0x3F);
            i += 3;
        } elif ((byte & 0xF8) == 0xF0) {
            if (i+3 >= utf8.size()){ return nullptr; }
            codepoint = static_cast<uchar_32>(byte & 0x07) << 18 | (utf8[i + 1] & 0x3F) << 12 | (utf8[i + 2] & 0x3F) << 6 | (utf8[i + 3] & 0x3F);
            i += 4;
        } else { return nullptr; }

        if (codepoint <= 0xFFFF) {
            utf16.push(type::cast<uchar_16>(codepoint));
        } elif (codepoint <= 0x10FFFF) {
            codepoint -= 0x10000;
            utf16.push(type::cast<uchar_16>((codepoint >>   10) + 0xD800));
            utf16.push(type::cast<uchar_16>((codepoint & 0x3FF) + 0xDC00));
        } else { return nullptr; }

    }   return utf16.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

inline ptr_t<uchar_8> utf16_to_utf8(const ptr_t<uchar_16>& utf16) {
    queue_t<uchar_8> utf8; ulong i = 0; while ( i<utf16.size() ) {
        uchar_32 codepoint = 0; uchar_16 unit = utf16[i];

        if (unit < 0xD800 || unit > 0xDFFF) { 
            codepoint = unit; i += 1;
        } elif (unit >= 0xD800 && unit <= 0xDBFF) {
            if (i + 1 >= utf16.size()){ return nullptr; }
            uchar_16 low = utf16[i + 1];
            if (low < 0xDC00 || low > 0xDFFF){ return nullptr; }
            codepoint = ((unit - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
            i += 2;
        } else { return nullptr; }

        if (codepoint <= 0x7F) {
            utf8.push( type::cast<uchar_8>(codepoint) );
        } elif (codepoint <= 0x7FF) {
            utf8.push( type::cast<uchar_8>((codepoint >>   6) | 0xC0) );
            utf8.push( type::cast<uchar_8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0xFFFF) {
            utf8.push( type::cast<uchar_8>((codepoint  >> 12) | 0xE0) );
            utf8.push( type::cast<uchar_8>(((codepoint >>  6) & 0x3F) | 0x80) );
            utf8.push( type::cast<uchar_8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0x10FFFF) {
            utf8.push( type::cast<uchar_8>((codepoint  >> 18) | 0xF0) );
            utf8.push( type::cast<uchar_8>(((codepoint >> 12) & 0x3F) | 0x80) );
            utf8.push( type::cast<uchar_8>(((codepoint >>  6) & 0x3F) | 0x80) );
            utf8.push( type::cast<uchar_8>((codepoint & 0x3F) | 0x80) );
        } else { return nullptr; }

    }   return utf8.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

inline ptr_t<uchar_32> utf16_to_utf32( const ptr_t<uchar_16>& utf16 ) {
    queue_t<uchar_32> utf32; ulong i = 0; while ( i<utf16.size() ) {
        uchar_16 unit = utf16[i];

        if( unit < 0xD800 || unit > 0xDFFF ) {
            utf32.push(type::cast<uchar_32>(unit)); i += 1;
        } elif ( unit >= 0xD800 && unit <= 0xDBFF ) {
            if ( i+1>=utf16.size() ){ return nullptr; }
            uchar_16 low = utf16[i + 1];
            if ( low<0xDC00 || low>0xDFFF){ return nullptr; }
            uchar_32 codepoint = ((unit-0xD800)<<10) + (low-0xDC00) + 0x10000;
            utf32.push( codepoint ); i += 2;
        } else { return nullptr; }
    
    }   return utf32.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

inline ptr_t<uchar_16> utf32_to_utf16( const ptr_t<uchar_32>& utf32 ) {
    queue_t<uchar_16> utf16; for( uchar_32 codepoint : utf32 ) {

        if (codepoint <= 0xFFFF) {
            utf16.push( type::cast<uchar_16>(codepoint) );
        } elif (codepoint <= 0x10FFFF) { codepoint -= 0x10000;
            uchar_16 high_surrogate = type::cast<uchar_16>((codepoint >>   10) + 0xD800);
            uchar_16 low_surrogate  = type::cast<uchar_16>((codepoint & 0x3FF) + 0xDC00);
            utf16.push( high_surrogate ); utf16.push( low_surrogate );
        } else { return nullptr; }

    }   return utf16.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

}} 

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/