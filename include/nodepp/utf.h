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

ptr_t<uint32> utf8_to_utf32( const ptr_t<uint8>& utf8 ) {
    queue_t<uint32> utf32; ulong i=0; while ( i<utf8.size() ) {
        uint32 codepoint = 0; uint8 byte = utf8[i];

        if (byte < 0x80) {
            codepoint = byte; i += 1;
        } elif ((byte & 0xE0) == 0xC0) {
            if (i + 1 >= utf8.size()){ throw except_t("Invalid UTF-8 sequence"); return nullptr; }
            codepoint = (byte & 0x1F) << 6 | (utf8[i + 1] & 0x3F);
            i += 2;
        } elif ((byte & 0xF0) == 0xE0) {
            if (i + 2 >= utf8.size()){ throw except_t("Invalid UTF-8 sequence"); return nullptr; }
            codepoint = (byte & 0x0F) << 12 | (utf8[i + 1] & 0x3F) << 6 | (utf8[i + 2] & 0x3F);
            i += 3;
        } elif ((byte & 0xF8) == 0xF0) {
            if (i + 3 >= utf8.size()){ throw except_t("Invalid UTF-8 sequence"); return nullptr; }
            codepoint = (byte & 0x07) << 18 | (utf8[i + 1] & 0x3F) << 12 | (utf8[i + 2] & 0x3F) << 6 | (utf8[i + 3] & 0x3F);
            i += 4;
        } else { throw except_t("Invalid UTF-8 byte"); return nullptr; }

        utf32.push( codepoint );
    }   return utf32.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

ptr_t<uint8> utf32_to_utf8( const ptr_t<uint32>& utf32 ) {
    queue_t<uint8> utf8; for ( uint32 codepoint : utf32 ) {
        if (codepoint <= 0x7F) {
            utf8.push( type::cast<uint8>(codepoint) );
        } elif (codepoint <= 0x7FF) {
            utf8.push( type::cast<uint8>((codepoint >>   6) | 0xC0) );
            utf8.push( type::cast<uint8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0xFFFF) {
            utf8.push( type::cast<uint8>((codepoint  >> 12) | 0xE0) );
            utf8.push( type::cast<uint8>(((codepoint >>  6) & 0x3F) | 0x80) );
            utf8.push( type::cast<uint8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0x10FFFF) {
            utf8.push( type::cast<uint8>((codepoint >> 18)  | 0xF0) );
            utf8.push( type::cast<uint8>(((codepoint >> 12) & 0x3F) | 0x80) );
            utf8.push( type::cast<uint8>(((codepoint >> 6)  & 0x3F) | 0x80) );
            utf8.push( type::cast<uint8>((codepoint & 0x3F) | 0x80) );
        } else { 
            throw except_t("Invalid UTF-32 codepoint"); return nullptr;
        }
    }   return utf8.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

ptr_t<uint16> utf8_to_utf16( const ptr_t<uint8>& utf8 ) {
    queue_t<uint16> utf16; ulong i = 0; while ( i<utf8.size() ) {
        uint32 codepoint = 0; uint8 byte = utf8[i];

        if (byte < 0x80) {
            codepoint = byte; i += 1;
        } elif ((byte & 0xE0) == 0xC0) {
            if (i+1 >= utf8.size()){ throw except_t("Invalid UTF-8 sequence"); return nullptr; }
            codepoint = (byte & 0x1F) << 6 | (utf8[i + 1] & 0x3F); 
            i += 2;
        } elif ((byte & 0xF0) == 0xE0) {
            if (i+2 >= utf8.size()){ throw except_t("Invalid UTF-8 sequence"); return nullptr; }
            codepoint = (byte & 0x0F) << 12 | (utf8[i + 1] & 0x3F) << 6 | (utf8[i + 2] & 0x3F);
            i += 3;
        } elif ((byte & 0xF8) == 0xF0) {
            if (i+3 >= utf8.size()){ throw except_t("Invalid UTF-8 sequence"); return nullptr; }
            codepoint = (byte & 0x07) << 18 | (utf8[i + 1] & 0x3F) << 12 | (utf8[i + 2] & 0x3F) << 6 | (utf8[i + 3] & 0x3F);
            i += 4;
        } else { throw except_t("Invalid UTF-8 sequence"); return nullptr; }

        if (codepoint <= 0xFFFF) {
            utf16.push(type::cast<uint16>(codepoint));
        } elif (codepoint <= 0x10FFFF) {
            codepoint -= 0x10000;
            utf16.push(type::cast<uint16>((codepoint >>   10) + 0xD800));
            utf16.push(type::cast<uint16>((codepoint & 0x3FF) + 0xDC00));
        } else { throw except_t("Invalid Unicode codepoint"); return nullptr; }

    }   return utf16.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

ptr_t<uint8> utf16_to_utf8(const ptr_t<uint16>& utf16) {
    queue_t<uint8> utf8; ulong i = 0; while ( i<utf16.size() ) {
        uint32 codepoint = 0; uint16 unit = utf16[i];

        if (unit < 0xD800 || unit > 0xDFFF) { 
            codepoint = unit; i += 1;
        } elif (unit >= 0xD800 && unit <= 0xDBFF) {
            if (i + 1 >= utf16.size()){ throw except_t("Invalid UTF-16 sequence"); return nullptr; }
            uint16 low = utf16[i + 1];
            if (low < 0xDC00 || low > 0xDFFF){ throw except_t("Invalid UTF-16 low surrogate"); return nullptr; }
            codepoint = ((unit - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
            i += 2;
        } else { throw except_t("Invalid UTF-16 high surrogate"); return nullptr; }

        if (codepoint <= 0x7F) {
            utf8.push( type::cast<uint8>(codepoint) );
        } elif (codepoint <= 0x7FF) {
            utf8.push( type::cast<uint8>((codepoint >>   6) | 0xC0) );
            utf8.push( type::cast<uint8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0xFFFF) {
            utf8.push( type::cast<uint8>((codepoint  >> 12) | 0xE0) );
            utf8.push( type::cast<uint8>(((codepoint >>  6) & 0x3F) | 0x80) );
            utf8.push( type::cast<uint8>((codepoint & 0x3F) | 0x80) );
        } elif (codepoint <= 0x10FFFF) {
            utf8.push( type::cast<uint8>((codepoint  >> 18) | 0xF0) );
            utf8.push( type::cast<uint8>(((codepoint >> 12) & 0x3F) | 0x80) );
            utf8.push( type::cast<uint8>(((codepoint >>  6) & 0x3F) | 0x80) );
            utf8.push( type::cast<uint8>((codepoint & 0x3F) | 0x80) );
        } else { throw except_t("Invalid Unicode codepoint"); return nullptr; }

    }   return utf8.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

ptr_t<uint32> utf16_to_utf32( const ptr_t<uint16>& utf16 ) {
    queue_t<uint32> utf32; ulong i = 0; while ( i<utf16.size() ) {
        uint16 unit = utf16[i];

        if( unit < 0xD800 || unit > 0xDFFF ) {
            utf32.push(type::cast<uint32>(unit)); i += 1;
        } elif ( unit >= 0xD800 && unit <= 0xDBFF ) {
            if ( i+1>=utf16.size() ) { throw except_t("Invalid UTF-16 sequence"); return nullptr; }
            uint16 low = utf16[i + 1];
            if ( low<0xDC00 || low>0xDFFF) { throw except_t("Invalid UTF-16 low surrogate"); return nullptr; }
            uint32 codepoint = ((unit-0xD800)<<10) + (low-0xDC00) + 0x10000;
            utf32.push( codepoint ); i += 2;
        } else { throw except_t("Invalid UTF-16 high surrogate"); return nullptr; }
    
    }   return utf32.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

ptr_t<uint16> utf32_to_utf16( const ptr_t<uint32>& utf32 ) {
    queue_t<uint16> utf16; for( uint32 codepoint : utf32 ) {

        if (codepoint <= 0xFFFF) {
            utf16.push( type::cast<uint16>(codepoint) );
        } elif (codepoint <= 0x10FFFF) { codepoint -= 0x10000;
            uint16 high_surrogate = type::cast<uint16>((codepoint >>   10) + 0xD800);
            uint16 low_surrogate  = type::cast<uint16>((codepoint & 0x3FF) + 0xDC00);
            utf16.push( high_surrogate ); utf16.push( low_surrogate );
        } else { throw except_t("Invalid Unicode codepoint"); return nullptr; }

    }   return utf16.data();
}

/*────────────────────────────────────────────────────────────────────────────*/

}} 
#endif

/*────────────────────────────────────────────────────────────────────────────*/