/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_REGEX
#define NODEPP_REGEX

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class regex_t : public generator_t {
protected:

    struct REGEX {
        uchar /*----*/ flag, data;
        queue_t<REGEX> next;
        ptr_t<int>     rep;
    };

    struct NODE {
        REGEX queue; bool icase=0;
        queue_t<string_t> memory ;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    ptr_t<int> get_next_repeat( const string_t& pattern, ulong& off ) const noexcept {

        if( pattern[off] == '{' ){
            auto end = get_next_key( pattern, off ); /*----------*/
            auto out = get_rep( pattern, off, end ); off = end + 1;
        return out; }

        elif( pattern[off]=='?' ){ return ptr_t<int>({ 0, 1 }); ++off; }
        elif( pattern[off]=='*' ){ return ptr_t<int>({ 0,-1 }); ++off; }
        elif( pattern[off]=='+' ){ return ptr_t<int>({ 1,-1 }); ++off; }

    return nullptr; }

    ptr_t<int> get_rep( const string_t& pattern, int start, int end ) const noexcept {
    ptr_t<int>     rep({ 0, 0 }); bool b=0; string_t num[2];

        pattern.slice( start+1, end ).map([&]( char& data ){
            if  (!string::is_digit(data) ){ b =! b; }
            elif( string::is_digit(data) ){ num[b].push(data); }
        });

        if( !num[0].empty() ){ rep[0] =string::to_int(num[0]);
        /*------------------*/ rep[1] =string::to_int(num[0]); }
        if( !num[1].empty() ){ rep[1] =string::to_int(num[1]); }

    return rep; }

    ptr_t  <int> get_next_regex( const string_t& pattern, ulong off ) const noexcept {
    queue_t<int> out;

        while( off < pattern.size() ){
           if( pattern[off] == '|' ){ out.push(off); off++; }
           if( pattern[off] == '[' || /*-------------------*/
               pattern[off] == '{' || /*-------------------*/
               pattern[off] == '(' // /*-------------------*/
           ) { off = get_next_key( pattern,off ); continue; }
           if( pattern[off] == '\\'){ ++off; } /*-*/ ++off;
        }

    out.push( pattern.size() ); return out.data(); }

    inline int get_next_key( const string_t& pattern, ulong off ) const noexcept {
        uchar k=0; while( off < pattern.size() ){

            switch( pattern[off] ){ /**/ case '\\': ++off ; break;
                case '[': k += 1; break; case ']' : k -= 1; break;
                case '(': k += 2; break; case ')' : k -= 2; break;
                case '{': k += 3; break; case '}' : k -= 3; break;
            }

            if( k == 0 ){ break; } /*------*/ ++off;
        }   return off == pattern.size() ? -1 : off;
    }

    /*─······································································─*/

    REGEX compile_range( const string_t& pattern ) const {
    REGEX node; auto next = &node.next;

        for( ulong x=0; x<pattern.size(); ++x ){
        if ( pattern[x] == '\\' ){ /*--*/ ++x;
        if ( x < pattern.size() ){ REGEX item; item.flag=0x00; item.data=pattern[x]; next->push(item); }
        } elif ( pattern[x+1]=='-' && (x+2)<pattern.size() ){
            auto a = min( pattern[x], pattern[x+2] ); /*---*/
            auto b = max( pattern[x], pattern[x+2] ); x += 2;
            for( uchar y=a; y<=b; ++y )
               { REGEX item; item.flag=0x00; item.data=y; /*----*/ next->push(item); }
        } else { REGEX item; item.flag=0x00; item.data=pattern[x]; next->push(item); }}

    return node; }

    /*─······································································─*/

    REGEX compile_pattern( const string_t& pattern ) const {
    ulong off=0; REGEX node; node.data=0x00; node.flag=0x09;
    do { /*---*/ REGEX item; item.data=0x00; item.flag=0xff;

        if( pattern[off] == ']' || pattern[off] == '{' ||
            pattern[off] == '}' || pattern[off] == ')' // regex error handling
        ) { throw except_t(string::format( "regex: %d %c", off, pattern[off] )); }

        elif( pattern[off]=='[' ){
        int end=0; if(( end=get_next_key(pattern,off) )==-1 ){ /*-------------*/
            throw except_t(string::format( "regex: %d %c", off, pattern[off] ));
        }   int beg = pattern[off+1]=='^'? off+2:off+1;
            item=compile_range(pattern.slice(beg,end));
            item.data=pattern[off+1]=='^'? 0xff : 0x00;
            item.flag=0x08; off=end; /*--------------*/
        }

        elif( pattern[off]== '(' ){
        int end=0; if(( end=get_next_key(pattern,off) )==-1 ){ /*-------------*/
            throw except_t(string::format( "regex: %d %c", off, pattern[off] ));
        }   item=compile(pattern.slice( off+1, end ));
            item.data=0xff; item.flag=0x09; off=end;
        }

        elif( pattern[off] == '$' ){ item.data=0x00; item.flag=0x01; }
        elif( pattern[off] == '^' ){ item.data=0x00; item.flag=0x02; }
        elif( pattern[off] == '.' ){ item.data=0x00; item.flag=0x0c; }

        elif( pattern[off] == '\\'){ ++off; /*---------------------*/
          if( pattern[off] == 'b' ){ item.data=0xff; item.flag=0x03; }
        elif( pattern[off] == 'B' ){ item.data=0x00; item.flag=0x03; }
        elif( pattern[off] == 'w' ){ item.data=0xff; item.flag=0x04; }
        elif( pattern[off] == 'W' ){ item.data=0x00; item.flag=0x04; }
        elif( pattern[off] == 'd' ){ item.data=0xff; item.flag=0x05; }
        elif( pattern[off] == 'D' ){ item.data=0x00; item.flag=0x05; }
        elif( pattern[off] == 's' ){ item.data=0xff; item.flag=0x06; }
        elif( pattern[off] == 'S' ){ item.data=0x00; item.flag=0x06; }
        elif( pattern[off] == 'n' ){ item.data=0xff; item.flag=0x07; }
        elif( pattern[off] == 'N' ){ item.data=0x00; item.flag=0x07; }

        else{ item.data=pattern[off]; item.flag=0x00; }}
        else{ item.data=pattern[off]; item.flag=0x00; }

        if( pattern[off+1] == '+' || pattern[off+1] == '?' ||
            pattern[off+1] == '*' || pattern[off+1] == '{' //
        ) { off++; item.rep=get_next_repeat( pattern,off ); }

        node.next.push(item);

    } while( ++off < pattern.size() ); return node; }

    /*─······································································─*/

    REGEX compile( const string_t& pattern ) const {
    REGEX node; node.data=0x00; node.flag=0x09;
    do{ if( pattern.empty() ){ break; }

        auto reg =get_next_regex( pattern,0 );
        auto addr=reg.begin(); ulong x=0, y=0;

        while( addr != reg.end() ){ y=*addr;
        node.next.push( compile_pattern(pattern.slice(x,y)) );
        x=*addr+1; ++addr; } /*-----------------------------*/

    } while(0); return node; }

    /*─······································································─*/

    int _search( string_t value, ulong offset, REGEX item ) const noexcept {
    int rep=0; ulong tmp=offset;

        /*─·································································─*/

        if  ( item.flag==0x02 && item.data==0x00 ){
        if  ( offset == 0 ) /*-----*/ { return 0; } return -1; }

        elif( item.flag==0x01 && item.data==0x00 ){
        if  ( offset >= value.last() ){ return 0; } return -1; }

        /*─·································································─*/

        if  ( item.flag==0x0c && item.data==0x00 ){ return 1; }

        elif( item.flag==0x03 && item.data==0x00 ){
        if(!( offset==0|| offset>=value.last() ) ){ return 1; }}

        elif( item.flag==0x03 && item.data==0xff ){
        if  ( offset==0|| offset>=value.last()   ){ return 1; }}

        elif( item.flag==0x04 && item.data==0x00 ){
        if(!( string::is_alnum( value[offset] )) ){ return 1; }}

        elif( item.flag==0x04 && item.data==0xff ){
        if  ( string::is_alnum( value[offset] )  ){ return 1; }}

        elif( item.flag==0x05 && item.data==0x00 ){
        if(!( string::is_digit( value[offset] ))){ return 1; }}

        elif( item.flag==0x05 && item.data==0xff ){
        if  ( string::is_digit( value[offset] )  ){ return 1; }}

        elif( item.flag==0x06 && item.data==0x00 ){
        if(!( string::is_space( value[offset] ))){ return 1; }}

        elif( item.flag==0x06 && item.data==0xff ){
        if  ( string::is_space( value[offset] )  ){ return 1; }}

        elif( item.flag==0x07 && item.data==0x00 ){
        if(!( string::is_print( value[offset] )) ){ return 1; }}

        elif( item.flag==0x07 && item.data==0xff ){
        if  ( string::is_print( value[offset] )  ){ return 1; }}

        /*─·································································─*/

        elif( item.flag==0x08 && item.data==0xff && !obj->icase ){
        if  ( item.next.none([&]( REGEX x ){
              return x.data==value[offset];
        }) ){ return 1; }}

        elif( item.flag==0x08 && item.data==0x00 && !obj->icase ){
        if  ( item.next.some([&]( REGEX x ){
              return x.data==value[offset];
        }) ){ return 1; }}

        /*─·································································─*/

        elif( item.flag==0x08 && item.data==0xff && obj->icase ){
        if  ( item.next.none([&]( REGEX x ){
              return string::to_lower(x.data)==string::to_lower(value[offset]);
        }) ){ return 1; }}

        elif( item.flag==0x08 && item.data==0x00 && obj->icase ){
        if  ( item.next.some([&]( REGEX x ){
              return string::to_lower(x.data)==string::to_lower(value[offset]);
        }) ){ return 1; }}

        /*─·································································─*/

        elif( item.flag==0x00 ){
            char a = obj->icase? string::to_lower(item.data)/**/ : item.data;
            char b = obj->icase? string::to_lower(value[offset]) : value[offset];
        return a == b ? 1 : -1; }

        /*─·································································─*/

        elif( item.flag==0x09 && item.data==0xff ){ auto x=item.next.first();
        while( x != nullptr ){ /*----------------*/ auto y=x->next;
        /*---------------------------------------*/ auto z=_search( value, offset, x->data );
          if  ( z==0 ){ /*-------------------------------------------------*/ x=y; continue; }
          elif( z>=1 ){ obj->memory.push(value.slice(tmp, tmp+z)); offset=tmp +z ; return z; }
          else/*----*/{ /*--------------------------------------*/ offset=tmp;x=y; continue; }
        } return -1; }

        /*─·································································─*/

        else{ auto x = item.next.first(); int out=0; while( x != nullptr ){

        if  ( offset>value.size() ){
        if  ( x->next != nullptr  ){ return -1; }
        else /*-----------------*/ { break/**/; }}

        /*-------------*/ auto z =_search( value, offset, x->data );
        if ( z>= 1 ){ offset +=z; } auto y=x->next; ++rep;
        /**/ out = type::cast<int>( offset - tmp );

        if  ( x->data.data==0x00 && x->data.flag==0x09 && z>=1 ){ return out; }

        if  ( z>=1 && !x->data.rep.null() ){
        if  ( -1 ==x->data.rep[1] ){ continue ; }
        elif( rep>=x->data.rep[1] ){ goto NEXT; }
        else /*------------------*/{ continue ; }}

        elif( z<0 && !x->data.rep.null() ) {
        if  ( -1==x->data.rep[1] ){
        if  ( rep>x->data.rep[0] ){ goto NEXT; } /*-----*/ return  0; }
        elif( rep>x->data.rep[0] && rep<=x->data.rep[1] ){ goto NEXT; }
        else /*----------------------------------------*/{ return  0; }}

        if  ( z<0 ){ return 0; }

        NEXT:; x=y; rep=0; } return out; }

        /*─·································································─*/

    return -1; }

public:

    virtual ~regex_t() noexcept { clear_memory(); }

    regex_t (): obj( new NODE() ){}

    regex_t ( const string_t& reg, bool icase=false ): obj( new NODE() )
    /*---*/ { obj->icase=icase; obj->queue=compile(reg); }

    /*─······································································─*/

    void clear_memory() /*------*/ const noexcept { obj->memory.clear(); }

    array_t<string_t> get_memory() const noexcept { return obj->memory.data(); }

    /*─······································································─*/

    ptr_t<ulong> search( string_t value, ulong off=0 ){
    ptr_t<ulong> range({ 0, 0 }); while( off < value.size() ) {
        int c=0; if((c=_search( value, off, obj->queue )) <=0 )
        { ++off; continue; } /*------------------------------*/
        range[0]=off; range[1]=off+type::cast<ulong>(c); break;
    } return range[0]==range[1] ? nullptr : range; }

    /*─······································································─*/

    array_t<ptr_t<ulong>> search_all( const string_t& _str ){
        queue_t<ptr_t<ulong>> out; ulong off=0; for(;;) {
            auto idx = search( _str, off );
            if( idx.null() /**/ ){ return out.data(); }
            if( idx[0]==idx[1]  ){ return out.data(); } off =idx[1];
                ptr_t<ulong> mem({ idx[0], idx[1] }); out.push(mem);
        }   return nullptr;
    }

    /*─······································································─*/

    array_t<string_t> split( const string_t& _str ){ ulong n = 0;
        auto idx = search_all( _str ); queue_t<string_t> out;
        if ( idx.empty()  ){ out.push(_str); return out.data(); }
        for( auto x : idx ){
             out.push( _str.slice( n, x[0] ) ); n = x[1];
        }    out.push( _str.slice( n ) ); return out.data();
    }

    /*─······································································─*/

    string_t replace_all( string_t _str, const string_t& _rep ){
        auto idx = search_all( _str ).reverse(); for( auto x : idx ){
             _str.splice( x[0], x[1] - x[0], _rep );
        }    return _str;
    }

    string_t replace( string_t _str, const string_t& _rep, ulong off=0 ){
        auto idx = search( _str, off ); /*----------------------*/
        if( idx.null() /*-*/ ){ return _str; } /*---------------*/
        if( idx[0] == idx[1] ){ return _str; } /*---------------*/
        _str.splice( idx[0], idx[1] - idx[0], _rep ); return _str;
    }

    /*─······································································─*/

    string_t remove_all( string_t _str ){
        auto idx = search_all( _str ).reverse(); for( auto x : idx ){
             _str.splice( x[0], x[1] - x[0] );
        }    return _str;
    }

    string_t remove( string_t _str, ulong off=0 ){
        auto idx = search( _str, off ); /*----------------*/
        if( idx.null() /*-*/ ){ return _str; }  /*--------*/
        if( idx[0] == idx[1] ){ return _str; }  /*--------*/
        _str.splice( idx[0], idx[1] - idx[0] ); return _str;
    }

    /*─······································································─*/

    array_t<string_t> match_all( const string_t& _str ){
        auto idx = search_all( _str ); queue_t<string_t> out;
        for( auto x : idx ){ /*---------------*/
             out.push(_str.slice( x[0], x[1] ));
        }    return out.data();
    }

    /*─······································································─*/

    string_t match( const string_t& _str, ulong off=0 ){
        auto idx = search( _str, off ); /*-----*/
        if( idx.null() /*-*/ ){ return nullptr; }
        if( idx[0] == idx[1] ){ return nullptr; }
            return _str.slice( idx[0], idx[1] );
    }

    /*─······································································─*/

    bool test( const string_t& _str, ulong off=0 ){
        auto idx = search( _str, off );
        if( idx.null() /*-*/ ){ return 0; }
        if( idx[0] == idx[1] ){ return 0; }
        /*-------------------*/ return 1;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace regex {

    string_t remove( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.remove( _str );
    }

    /*─······································································─*/

    string_t replace_all( const string_t& _str, const string_t& _reg, const string_t& _rep, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.replace_all( _str, _rep );
    }

    /*─······································································─*/

    string_t remove_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.remove_all( _str );
    }

    /*─······································································─*/

    array_t<ptr_t<ulong>> search_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.search_all( _str );
    }

    /*─······································································─*/

    string_t replace( const string_t& _str, const string_t& _reg, const string_t& _rep, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.replace( _str, _rep );
    }

    /*─······································································─*/

    array_t<string_t> get_memory( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); reg.search_all( _str ); return reg.get_memory();
    }

    /*─······································································─*/

    array_t<string_t> match_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.match_all( _str );
    }

    /*─······································································─*/

    ptr_t<ulong> search( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.search( _str );
    }

    /*─······································································─*/

    string_t match( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.match( _str );
    }

    /*─······································································─*/

    bool test( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.test( _str );
    }

    /*─······································································─*/

    array_t<string_t> split( const string_t& _str, char ch ){ return string::split( _str, ch ); }

    array_t<string_t> split( const string_t& _str, int ch ){ return string::split( _str, ch ); }

    array_t<string_t> split( const string_t& _str, const string_t& _reg, bool _flg=false ){
          if ( _reg.size ()== 1 ){ return string::split( _str, _reg[0] ); }
        elif ( _reg.empty() )    { return string::split( _str, 1 ); }
        regex_t reg( _reg, _flg ); return reg.split( _str );
    }

    /*─······································································─*/

    template< class T, class... V >
    string_t join( const string_t& c, const T& argc, const V&... args ){
        return string::join( c, argc, args... );
    }

    /*─······································································─*/

    template< class... T >
    string_t format( const string_t& val, const T&... args ){
        auto count = string::count( []( string_t ){ return true; }, args... );
        queue_t<string_t> out; ulong idx=0;
        regex_t reg( "\\$\\{\\d+\\}" );

        for( auto &x: reg.search_all( val ) ){
        auto y = string::to_uint( regex::match( val.slice( x[0], x[1] ), "\\d+" ) );
        if ( y >= count ){ break; }
             out.push( val.slice( idx, x[0]    ) );
             out.push( string::get( y, args... ) ); idx = x[1];
        }    out.push( val.slice( idx ) );

        return array_t<string_t>( out.data() ).join("");
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
