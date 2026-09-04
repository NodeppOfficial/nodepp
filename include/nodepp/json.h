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

    ulong get_next_sec( ulong _pos, const string_t& str ) const noexcept {
        uchar k=0; while( _pos < str.size() && k < 128 ){
        switch( str[_pos] ){
            case ':': k += 6; break; case ',': k -= 6; break;
            case '{': _pos =get_next_key( _pos, str ); break;
            case '[': _pos =get_next_key( _pos, str ); break;
            case '"': _pos =get_next_key( _pos, str ); break;
            case '\\':_pos =get_next_key( _pos, str ); break;
            case '}':    k =0; break;  case ']': k =0; break;
        }   if( k == 0 ){ break; } 
    ++_pos; } return _pos; }

    ulong get_next_key( ulong _pos, const string_t& str ) const noexcept {
        bool x=1; uchar k=0; while( _pos < str.size() && k < 128 ){
        switch( str[_pos] ){
            case '[': k += 1; break; case ']': k -= 1; break;
            case '{': k += 3; break; case '}': k -= 3; break;
            case '\\':++_pos; break; case '"':
                if( x ){ k+=5; x=!x; }
                else   { k-=5; x=!x; }
            break;
        } if( k == 0 ){ break; } ++_pos; }
    return _pos; }

    object_t get_data( const string_t& data ) const noexcept {

        thread_local static ptr_t<regex_t> reg ({
               regex_t( "[a-z]"   ),
               regex_t( "[.]\\d+" ),
               regex_t( "\\d+"    )
        });

        ulong x=0; while( x < data.size() && data[x]==' ' ){ ++x; }

        if  ( data.empty() || data[x] == ',' ) /*---*/ { return nullptr; }
        elif( data[x] == '"'     ) /*---------------*/ { 
              return data.slice( x+1, get_next_sec( x, data) ); }
        elif( data[x] == '{'     ) /*---------------*/ { return parse( data ); }
        elif( data[x] == '['     ) /*---------------*/ { return parse( data ); }
        elif( data.find("false") ) /*---------------*/ { return (bool) 0; }
        elif( data.find("true")  ) /*---------------*/ { return (bool) 1; }
        elif( data.find("null")  ) /*---------------*/ { return nullptr ; }
        elif( reg[0].test(data)  ) /*---------------*/ { return (string_t) data; }
        elif( data.find('.')     ) /*---------------*/ {
            if  ( reg[1].match(data).size()>5 ) /*--*/ { return string::to_double(data); }
            else /*---------------------------------*/ { return string::to_float (data); }
        }   elif( reg[2].match(data).size()>9 ) /*--*/ { return string::to_u64   (data); }
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
    } while( x++<y ); return out.keys().empty() ? nullptr : out; }

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
    } while( x++<y ); return data.data(); }

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
                if( string::is_space( str[x]) )
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

            case 0x0001: return string::to_string( obj.as<int>      () );      break;
            case 0x0002: return string::to_string( obj.as<uint>     () );      break;
            case 0x0003: return obj.as<bool>() ? "true" : "false";             break;
            case 0x0004: return string::format("\"%c\"",obj.as<char>());       break;
            case 0x0005: return string::to_string( obj.as<long>     () );      break;
            case 0x0006: return string::to_string( obj.as<short>    () );      break;
            case 0x0007: return string::to_string( obj.as<uchar>    () );      break;
            case 0x0008: return string::to_string( obj.as<llong>    () );      break;
            case 0x0009: return string::to_string( obj.as<ulong>    () );      break;
            case 0x000A: return string::to_string( obj.as<ushort>   () );      break;
            case 0x000B: return string::to_string( obj.as<ullong>   () );      break;
            case 0x000C: return string::to_string( obj.as<wchar_t>  () );      break;
            case 0x000D: return string::to_string( obj.as<float>    () );      break;
            case 0x000E: return string::to_string( obj.as<double>   () );      break;
            case 0x000F: return string::to_string( obj.as<ldouble>  () );      break;
            case 0x0010: return regex ::format("\"${0}\"",obj.as<string_t>()); break;

            case 0xFA03: do { out.push('[');
            for( auto &x: obj.as<array_t<bool>>() )
               { out += regex::format("\"${0}\",",x ? "true":"false" ); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xFA04: do { out.push('[');
            for( auto &x: obj.as<array_t<char>>() )
               { out += string::format("\"%c\",", x ); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xFA10: do { out.push('[');
            for( auto &x: obj.as<array_t<string_t>>() )
               { out += regex::format("\"${0}\",", x.get() ); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xFA01: return regex::format("[${0}]",string::join(obj.as<array_t<int>>    ()) ); break;
            case 0xFA02: return regex::format("[${0}]",string::join(obj.as<array_t<uint>>   ()) ); break;
            case 0xFA05: return regex::format("[${0}]",string::join(obj.as<array_t<long>>   ()) ); break;
            case 0xFA06: return regex::format("[${0}]",string::join(obj.as<array_t<short>>  ()) ); break;
            case 0xFA07: return regex::format("[${0}]",string::join(obj.as<array_t<uchar>>  ()) ); break;
            case 0xFA08: return regex::format("[${0}]",string::join(obj.as<array_t<llong>>  ()) ); break;
            case 0xFA09: return regex::format("[${0}]",string::join(obj.as<array_t<ulong>>  ()) ); break;
            case 0xFA0A: return regex::format("[${0}]",string::join(obj.as<array_t<ushort>> ()) ); break;
            case 0xFA0B: return regex::format("[${0}]",string::join(obj.as<array_t<ullong>> ()) ); break;
            case 0xFA0C: return regex::format("[${0}]",string::join(obj.as<array_t<wchar_t>>()) ); break;
            case 0xFA0D: return regex::format("[${0}]",string::join(obj.as<array_t<float>>  ()) ); break;
            case 0xFA0E: return regex::format("[${0}]",string::join(obj.as<array_t<double>> ()) ); break;
            case 0xFA0F: return regex::format("[${0}]",string::join(obj.as<array_t<ldouble>>()) ); break;

            case 0xF603: do { out.push('[');
            for( auto &x: obj.as<ptr_t<bool>>() )
               { out += regex::format("\"${0}\",",x ? "true":"false" ); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xF604: do { out.push('[');
            for( auto &x: obj.as<ptr_t<char>>() )
               { out += string::format("\"%c\",", x ); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xF610: do { out.push('[');
            for( auto &x: obj.as<ptr_t<string_t>>() )
               { out += regex::format("\"${0}\",", x.get() ); }
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xF601: return regex::format("[${0}]",string::join(obj.as<ptr_t<int>>    ()) ); break;
            case 0xF602: return regex::format("[${0}]",string::join(obj.as<ptr_t<uint>>   ()) ); break;
            case 0xF605: return regex::format("[${0}]",string::join(obj.as<ptr_t<long>>   ()) ); break;
            case 0xF606: return regex::format("[${0}]",string::join(obj.as<ptr_t<short>>  ()) ); break;
            case 0xF607: return regex::format("[${0}]",string::join(obj.as<ptr_t<uchar>>  ()) ); break;
            case 0xF608: return regex::format("[${0}]",string::join(obj.as<ptr_t<llong>>  ()) ); break;
            case 0xF609: return regex::format("[${0}]",string::join(obj.as<ptr_t<ulong>>  ()) ); break;
            case 0xF60A: return regex::format("[${0}]",string::join(obj.as<ptr_t<ushort>> ()) ); break;
            case 0xF60B: return regex::format("[${0}]",string::join(obj.as<ptr_t<ullong>> ()) ); break;
            case 0xF60C: return regex::format("[${0}]",string::join(obj.as<ptr_t<wchar_t>>()) ); break;
            case 0xF60D: return regex::format("[${0}]",string::join(obj.as<ptr_t<float>>  ()) ); break;
            case 0xF60E: return regex::format("[${0}]",string::join(obj.as<ptr_t<double>> ()) ); break;
            case 0xF60F: return regex::format("[${0}]",string::join(obj.as<ptr_t<ldouble>>()) ); break;

            case 0xF803: do { out.push('[');
            obj.as<queue_t<bool>>().map([&]( bool x )
               { out += regex::format("\"${0}\",",x ? "true":"false" ); });
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xF804: do { out.push('[');
            obj.as<queue_t<char>>().map([&]( char x )
               { out += string::format("\"%c\",", x ); });
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xF810: do { out.push('[');
            obj.as<queue_t<string_t>>().map([&]( string_t x )
               { out += regex::format("\"${0}\",", x.get() ); });
            if ( out[ out.size()-1 ] == ',' ){ out.pop(); }
            out.push(']'); } while(0); break;

            case 0xF801: return regex::format("[${0}]",string::join(obj.as<queue_t<int>>    ()) ); break;
            case 0xF802: return regex::format("[${0}]",string::join(obj.as<queue_t<uint>>   ()) ); break;
            case 0xF805: return regex::format("[${0}]",string::join(obj.as<queue_t<long>>   ()) ); break;
            case 0xF806: return regex::format("[${0}]",string::join(obj.as<queue_t<short>>  ()) ); break;
            case 0xF807: return regex::format("[${0}]",string::join(obj.as<queue_t<uchar>>  ()) ); break;
            case 0xF808: return regex::format("[${0}]",string::join(obj.as<queue_t<llong>>  ()) ); break;
            case 0xF809: return regex::format("[${0}]",string::join(obj.as<queue_t<ulong>>  ()) ); break;
            case 0xF80A: return regex::format("[${0}]",string::join(obj.as<queue_t<ushort>> ()) ); break;
            case 0xF80B: return regex::format("[${0}]",string::join(obj.as<queue_t<ullong>> ()) ); break;
            case 0xF80C: return regex::format("[${0}]",string::join(obj.as<queue_t<wchar_t>>()) ); break;
            case 0xF80D: return regex::format("[${0}]",string::join(obj.as<queue_t<float>>  ()) ); break;
            case 0xF80E: return regex::format("[${0}]",string::join(obj.as<queue_t<double>> ()) ); break;
            case 0xF80F: return regex::format("[${0}]",string::join(obj.as<queue_t<ldouble>>()) ); break;

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