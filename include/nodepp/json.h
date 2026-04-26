/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_JSON
#define NODEPP_JSON

/*────────────────────────────────────────────────────────────────────────────*/

#include "object.h"
#include "regex.h"
#include "map.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class json_t {
private:
    
    using T     = type::pair<string_t,object_t>;
    using QUEUE = map_t<string_t,object_t>;
    using ARRAY = array_t<object_t>;

protected:

    long get_next_sec( ulong _pos, const string_t& str ) const noexcept {
        uchar k=0; while( _pos < str.size() && k < 128 ){
            switch( str[_pos] ){
                case ':': k += 6; break; case ',': k -= 6; break;
                case '{': _pos =get_next_key( _pos, str ); break;
                case '[': _pos =get_next_key( _pos, str ); break;
                case '"': _pos =get_next_key( _pos, str ); break;
                case '\\':_pos =get_next_key( _pos, str ); break;
                case '}':    k =0; break;  case ']': k =0; break;
            }   if( k == 0 ){ break; } ++_pos;
        }   return _pos >= str.size() ? -1 : _pos;
    }

    long get_next_key( ulong _pos, const string_t& str ) const noexcept {
        bool x=1; uchar k=0; while( _pos < str.size() && k < 128 ){
            switch( str[_pos] ){
                case '[': k += 1; break; case ']': k -= 1; break;
                case '{': k += 3; break; case '}': k -= 3; break;
                case '\\':++_pos; break; case '"':
                    if( x ){ k+=5; x=!x; }
                    else   { k-=5; x=!x; }
                break;
            } if( k == 0 ){ break; } ++_pos;
        }   return _pos >= str.size() ? -1 : _pos;
    }

    object_t get_data( const string_t& data ) const noexcept {

        static ptr_t<regex_t> reg ({
               regex_t( "[a-z]"   ),
               regex_t( "[.]\\d+" ),
               regex_t( "\\d+"    )
        });

        ulong x=0; while( x < data.size() && data[x]==' ' ){ ++x; }

        if  ( data.empty() || data[x] == ',' ) /*---*/ { return nullptr; }
        elif( data[x] == '"'     ) /*---------------*/ { 
              return data.slice( x+1, get_next_sec( x, data ) ); }
        elif( data[x] == '{'     ) /*---------------*/ { return parse( data ); }
        elif( data[x] == '['     ) /*---------------*/ { return parse( data ); }
        elif( data.find("false") ) /*---------------*/ { return (bool) 0; }
        elif( data.find("true")  ) /*---------------*/ { return (bool) 1; }
        elif( data.find("null")  ) /*---------------*/ { return nullptr ; }
        elif( reg[0].test(data)  ) /*---------------*/ { return (string_t) data; }
        elif( data.find('.')     ) /*---------------*/ {
            if  ( reg[1].match(data).size()>5 ) /*--*/ { return string::to_double(data); }
            else /*---------------------------------*/ { return string::to_float (data); }
        }   elif( reg[2].match(data).size()>9 ) /*--*/ { return string::to_long  (data); }
            else /*---------------------------------*/ { return string::to_int   (data); }

    }

    object_t get_object( ulong x, ulong y, const string_t& str ) const {
        object_t out; do { type::pair<string_t,string_t> data;
           if( string::is_space(str[x]) ){ continue; }
           if( str[x] == '"' ){
               auto z = get_next_sec( x, str );
               data.first = str.slice( x+1,z );
            while( str[x]!=':' && x<y ){ ++x; }
               auto w = get_next_sec( x, str );
                    w = w<0 ? str.size()-1 : w;
               data.second = str.slice( x+1, w); x=w;
               out[data.first] = get_data( data.second );
            }
        } while( x++<y ); return out.keys().empty() ? nullptr : out;
    }

    ARRAY get_array( ulong x, ulong y, const string_t& str ) const {
        queue_t<object_t> data; do {
           if( string::is_space(str[x]) || str[x]==',' ){ continue; }
           if( str[x] == '{' || str[x] == '[' ){
               auto z = get_next_key( x, str );
           if( z < 0 ){ return nullptr; }
               data.push( parse(str.slice( x,z+1 )) ); x=z+1;
           } elif( str[x] == '"' ) {
               auto z = get_next_sec( x, str );
           if( z < 0 ){ return nullptr; }
               data.push( get_data(str.slice( x,z+1 )) ); x=z+1;
           } elif( x != y ) {
               ulong z=x; while( str[z]!=',' && z<y ) { ++z; }
               data.push( get_data(str.slice( x, z )) ); x=z;
           }
        } while( x++<y ); return data.data();
    }

public: json_t() noexcept {}

    object_t parse( const string_t& str ) const {
        if( str.empty() ){ return nullptr; }
        ulong x = 0; string_t data; do {

            if ( str[x] == '[' || str[x] == '{' || str[x] == '"' ){
                 auto pos = get_next_key( x, str );
            if ( pos < 0 ){ return nullptr; }

                if( str[x] == '[' ) {
                    return get_array( x+1, pos, str );
                } elif( str[x] == '{' ) {
                    return get_object( x+1,pos, str );
                } else {
                    data = str.slice( x+1, pos-1 ); break;
                }   x = pos + 1;

            } elif( str[x] == ']' || str[x] == '}' || str[x] == ')' ){
                return nullptr;
            } else {
                if( string::is_space( str[x] ) )
                  { continue; } data.push( str[x] );
            }

        } while ( x++<str.size() ); return get_data(data);
    }

    string_t format( const object_t& obj ) const {
        if( !obj.has_value() ){ return "{}"; } string_t out;

        if( obj.get_type_id() == 20 ){
            out.push('{');

            for( auto &item: obj.as<QUEUE>().data() ){
                 out += regex::format("\"${0}\":",item.first.get());
                 out += format( item.second ); out.push(',');
            }if( out[ out.size()-1 ] == ',' ){ out.pop(); }

            out.push('}'); goto END;
        } elif( obj.get_type_id() == 21 ){
            out.push('[');

            for( auto &item: obj.as<ARRAY>() )
               { out += format( item ); out.push(','); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }

            out.push(']'); goto END;
        }

        switch( obj.get_type_id() ){

            case 0x0001: return string::to_string( obj.as<int>() );                              break;
            case 0x0002: return string::to_string( obj.as<uint>() );                             break;
            case 0x0003: return obj.as<bool>() ? "true" : "false";                               break;
            case 0x0004: return string::format("\"%c\"",obj.as<char>());                         break;
            case 0x0005: return string::to_string( obj.as<long>() );                             break;
            case 0x0006: return string::to_string( obj.as<short>() );                            break;
            case 0x0007: return string::to_string( obj.as<uchar>() );                            break;
            case 0x0008: return string::to_string( obj.as<llong>() );                            break;
            case 0x0009: return string::to_string( obj.as<ulong>() );                            break;
            case 0x000a: return string::to_string( obj.as<ushort>() );                           break;
            case 0x000b: return string::to_string( obj.as<ullong>() );                           break;
            case 0x000c: return string::to_string( obj.as<wchar_t>() );                          break;
            case 0x000d: return string::to_string( obj.as<char16_t>() );                         break;
            case 0x000e: return string::to_string( obj.as<char32_t>() );                         break;
            case 0x000f: return string::to_string( obj.as<float>() );                            break;
            case 0x0010: return string::to_string( obj.as<double>() );                           break;
            case 0x0011: return string::to_string( obj.as<ldouble>() );                          break;
            case 0x0012: return regex ::format("\"${0}\"",obj.as<string_t>().get());             break;

            case 0xfA03: do { out.push('[');
             for( auto &x: obj.as<array_t<bool>>() )
                { out += regex::format("\"${0}\",",x ? "true":"false" ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xfA04: do { out.push('[');
             for( auto &x: obj.as<array_t<char>>() )
                { out += string::format("\"%c\",", x ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xfA12: do { out.push('[');
             for( auto &x: obj.as<array_t<string_t>>() )
                { out += regex::format("\"${0}\",", x.get() ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xfA01: return regex::format("[${0}]",obj.as<array_t<int>>()     .join().get()); break;
            case 0xfA02: return regex::format("[${0}]",obj.as<array_t<uint>>()    .join().get()); break;
            case 0xfA05: return regex::format("[${0}]",obj.as<array_t<long>>()    .join().get()); break;
            case 0xfA06: return regex::format("[${0}]",obj.as<array_t<short>>()   .join().get()); break;
            case 0xfA07: return regex::format("[${0}]",obj.as<array_t<uchar>>()   .join().get()); break;
            case 0xfA08: return regex::format("[${0}]",obj.as<array_t<llong>>()   .join().get()); break;
            case 0xfA09: return regex::format("[${0}]",obj.as<array_t<ulong>>()   .join().get()); break;
            case 0xfA0a: return regex::format("[${0}]",obj.as<array_t<ushort>>()  .join().get()); break;
            case 0xfA0b: return regex::format("[${0}]",obj.as<array_t<ullong>>()  .join().get()); break;
            case 0xfA0c: return regex::format("[${0}]",obj.as<array_t<wchar_t>>() .join().get()); break;
            case 0xfA0d: return regex::format("[${0}]",obj.as<array_t<char16_t>>().join().get()); break;
            case 0xfA0e: return regex::format("[${0}]",obj.as<array_t<char32_t>>().join().get()); break;
            case 0xfA0f: return regex::format("[${0}]",obj.as<array_t<float>>()   .join().get()); break;
            case 0xfA10: return regex::format("[${0}]",obj.as<array_t<double>>()  .join().get()); break;
            case 0xfA11: return regex::format("[${0}]",obj.as<array_t<ldouble>>() .join().get()); break;

            case 0xf603: do { out.push('[');
             for( auto &x: obj.as<ptr_t<bool>>() )
                { out += regex::format("\"${0}\",",x ? "true":"false" ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xf604: do { out.push('[');
             for( auto &x: obj.as<ptr_t<char>>() )
                { out += string::format("\"%c\",", x ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xf612: do { out.push('[');
             for( auto &x: obj.as<ptr_t<string_t>>() )
                { out += regex::format("\"${0}\",", x.get() ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xf601: return regex::format("[${0}]",array_t<int>(obj.as<ptr_t<int>>())          .join().get()); break;
            case 0xf602: return regex::format("[${0}]",array_t<uint>(obj.as<ptr_t<uint>>())        .join().get()); break;
            case 0xf605: return regex::format("[${0}]",array_t<long>(obj.as<ptr_t<long>>())        .join().get()); break;
            case 0xf606: return regex::format("[${0}]",array_t<short>(obj.as<ptr_t<short>>())      .join().get()); break;
            case 0xf607: return regex::format("[${0}]",array_t<uchar>(obj.as<ptr_t<uchar>>())      .join().get()); break;
            case 0xf608: return regex::format("[${0}]",array_t<llong>(obj.as<ptr_t<llong>>())      .join().get()); break;
            case 0xf609: return regex::format("[${0}]",array_t<ulong>(obj.as<ptr_t<ulong>>())      .join().get()); break;
            case 0xf60a: return regex::format("[${0}]",array_t<ushort>(obj.as<ptr_t<ushort>>())    .join().get()); break;
            case 0xf60b: return regex::format("[${0}]",array_t<ullong>(obj.as<ptr_t<ullong>>())    .join().get()); break;
            case 0xf60c: return regex::format("[${0}]",array_t<wchar_t>(obj.as<ptr_t<wchar_t>>())  .join().get()); break;
            case 0xf60d: return regex::format("[${0}]",array_t<char16_t>(obj.as<ptr_t<char16_t>>()).join().get()); break;
            case 0xf60e: return regex::format("[${0}]",array_t<char32_t>(obj.as<ptr_t<char32_t>>()).join().get()); break;
            case 0xf60f: return regex::format("[${0}]",array_t<float>(obj.as<ptr_t<float>>())      .join().get()); break;
            case 0xf610: return regex::format("[${0}]",array_t<double>(obj.as<ptr_t<double>>())    .join().get()); break;
            case 0xf611: return regex::format("[${0}]",array_t<ldouble>(obj.as<ptr_t<ldouble>>())  .join().get()); break;

            case 0xf803: do { out.push('[');
             for( auto &x: obj.as<queue_t<bool>>().data() )
                { out += regex::format("\"${0}\",",x ? "true":"false" ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xf804: do { out.push('[');
             for( auto &x: obj.as<queue_t<char>>().data() )
                { out += string::format("\"%c\",", x ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xf812: do { out.push('[');
             for( auto &x: obj.as<queue_t<string_t>>().data() )
                { out += regex::format("\"${0}\",", x.get() ); }
             if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xf801: return regex::format("[${0}]",array_t<int>(obj.as<queue_t<int>>().data())          .join().get()); break;
            case 0xf802: return regex::format("[${0}]",array_t<uint>(obj.as<queue_t<uint>>().data())        .join().get()); break;
            case 0xf805: return regex::format("[${0}]",array_t<long>(obj.as<queue_t<long>>().data())        .join().get()); break;
            case 0xf806: return regex::format("[${0}]",array_t<short>(obj.as<queue_t<short>>().data())      .join().get()); break;
            case 0xf807: return regex::format("[${0}]",array_t<uchar>(obj.as<queue_t<uchar>>().data())      .join().get()); break;
            case 0xf808: return regex::format("[${0}]",array_t<llong>(obj.as<queue_t<llong>>().data())      .join().get()); break;
            case 0xf809: return regex::format("[${0}]",array_t<ulong>(obj.as<queue_t<ulong>>().data())      .join().get()); break;
            case 0xf80a: return regex::format("[${0}]",array_t<ushort>(obj.as<queue_t<ushort>>().data())    .join().get()); break;
            case 0xf80b: return regex::format("[${0}]",array_t<ullong>(obj.as<queue_t<ullong>>().data())    .join().get()); break;
            case 0xf80c: return regex::format("[${0}]",array_t<wchar_t>(obj.as<queue_t<wchar_t>>().data())  .join().get()); break;
            case 0xf80d: return regex::format("[${0}]",array_t<char16_t>(obj.as<queue_t<char16_t>>().data()).join().get()); break;
            case 0xf80e: return regex::format("[${0}]",array_t<char32_t>(obj.as<queue_t<char32_t>>().data()).join().get()); break;
            case 0xf80f: return regex::format("[${0}]",array_t<float>(obj.as<queue_t<float>>().data())      .join().get()); break;
            case 0xf810: return regex::format("[${0}]",array_t<double>(obj.as<queue_t<double>>().data())    .join().get()); break;
            case 0xf811: return regex::format("[${0}]",array_t<ldouble>(obj.as<queue_t<ldouble>>().data())  .join().get()); break;

            default: return "{}"; break;
        }

        END:; return out;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace json {

    inline object_t parse ( const string_t& str ){ return json_t().parse ( str ); }

    inline string_t format( const object_t& obj ){ return json_t().format( obj ); }

    /*─······································································─*/

    template<class T, class V> 
    object_t parse( const map_t<T,V>& map ){ 
    object_t obj; for( auto &x: map.keys() )
          { obj[x] = map[x]; } return obj;
    }

    template<class T, class V>
    string_t format( const map_t<T,V>& map ){
    object_t obj; for( auto &x: map.data() ){
            obj[ x.first ] = x.second;
        }   return format( obj );
    }

    /*─······································································─*/

    template<class T, class V> 
    array_t<object_t> parse( const array_t<map_t<T,V>>& map ){
        queue_t<object_t> obj; for( auto &x: map )
        { obj.push( parse(x) ); } 
    return obj.data(); }

    template<class T, class V>
    string_t format( const array_t<map_t<T,V>>& map ){
    object_t obj = json::parse( map );
        return json::format( obj );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace json {
    template< class... T >
    string_t stringify( const T&... args ){ return format( args... ); }
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif