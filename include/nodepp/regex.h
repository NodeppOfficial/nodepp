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

#ifndef NODEPP_REGEX_ENGINE
#if ( _KERNEL_==NODEPP_KERNEL_ARDUINO ) || ( _KERNEL_==NODEPP_KERNEL_WASM )
    #define NODEPP_REGEX_ENGINE NODEPP_REGEX_LITE
#else
    #define NODEPP_REGEX_ENGINE NODEPP_REGEX_GRPH
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if ( NODEPP_REGEX_ENGINE == NODEPP_REGEX_GRPH )

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
            auto end = get_next_key( pattern, off ); /*------*/
            auto out = get_rep( pattern, off, end ); off = end;
        return out; }

        elif( pattern[off]=='?' ){ return ptr_t<int>({ 0, 1 }); }
        elif( pattern[off]=='*' ){ return ptr_t<int>({ 0,-1 }); }
        elif( pattern[off]=='+' ){ return ptr_t<int>({ 1,-1 }); }

    return nullptr; }

    ptr_t<int> get_rep( const string_t& pattern, int start, int end ) const noexcept {
    ptr_t<int>     rep({ 0, 0 }); 
    
        bool b=0; string_t num[2];

        pattern.slice_view( start+1, end ).map([&]( char& data ){
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

    ulong get_next_key( const string_t& pattern, ulong off ) const noexcept {
    uchar k=0; while( off < pattern.size() && k < 128 ){

        switch( pattern[off] ){ /**/ case '\\': ++off ; break;
            case '[': k += 1; break; case ']' : k -= 1; break;
            case '(': k += 2; break; case ')' : k -= 2; break;
            case '{': k += 3; break; case '}' : k -= 3; break;
        }   if( k==0 ) /**/ { break; }

    ++off; } return off; }

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
        ) { NODEPP_THROW_ERROR(string::format( "regex: %d %c", off, pattern[off] )); }

        elif( pattern[off]=='[' ){ ulong end=0; 
        if(( end=get_next_key(pattern,off) ) >= pattern.size() ){
            NODEPP_THROW_ERROR(string::format( "regex: %d %c", off, pattern[off] ));
        }   int beg = pattern[off+1]=='^'? off+2:off+1;
            item=compile_range(pattern.slice_view(beg,end));
            item.data=pattern[off+1]=='^'? 0xff : 0x00;
            item.flag=0x08; off=end; /*--------------*/
        }

        elif( pattern[off]== '(' ){ ulong end=0; 
        if(( end=get_next_key(pattern,off) ) >= pattern.size() ){
            NODEPP_THROW_ERROR(string::format( "regex: %d %c", off, pattern[off] ));
        }   item=compile(pattern.slice_view( off+1, end ));
            item.data=0xff; item.flag=0x09; off=end;
        }

        elif( pattern[off] == '$' ){ item.data=0x00; item.flag=0x01; }
        elif( pattern[off] == '^' ){ item.data=0x00; item.flag=0x02; }
        elif( pattern[off] == '.' ){ item.data=0x00; item.flag=0x0c; }

        elif( pattern[off] == '\\'){ ++off; /*---------------------*/
        if  ( pattern[off] == 'b' ){ item.data=0xff; item.flag=0x03; }
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
        node.next.push( compile_pattern(pattern.slice_view(x,y)) );
        x=*addr+1; ++addr; } /*---------------------------------*/

    } while(0); return node; }

    bool comparator( char a, char b ) const noexcept {
        return obj->icase ? ( string::to_lower(a)==string::to_lower(b) ) 
                          : ( a == b );
    }

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
        if(!( string::is_digit( value[offset] )) ){ return 1; }}

        elif( item.flag==0x05 && item.data==0xff ){
        if  ( string::is_digit( value[offset] )  ){ return 1; }}

        elif( item.flag==0x06 && item.data==0x00 ){
        if(!( string::is_space( value[offset] )) ){ return 1; }}

        elif( item.flag==0x06 && item.data==0xff ){
        if  ( string::is_space( value[offset] )  ){ return 1; }}

        elif( item.flag==0x07 && item.data==0x00 ){
        if(!( string::is_print( value[offset] )) ){ return 1; }}

        elif( item.flag==0x07 && item.data==0xff ){
        if  ( string::is_print( value[offset] )  ){ return 1; }}

        /*─·································································─*/

        elif( item.flag==0x08 && item.data==0xff ){
        if  ( item.next.none([&]( REGEX x ){
              return comparator( x.data, value[offset] );
        }) ){ return 1; }}

        elif( item.flag==0x08 && item.data==0x00 ){
        if  ( item.next.some([&]( REGEX x ){
              return comparator( x.data, value[offset] );
        }) ){ return 1; }}

        /*─·································································─*/

        elif( item.flag==0x00 ){ return comparator( item.data, value[offset] ) ? 1 : -1; }

        /*─·································································─*/

        elif( item.flag==0x09 && item.data==0xff ){ auto x=item.next.first();
        while( x != nullptr ){ /*----------------*/ auto y=x->next;
        /*---------------------------------------*/ auto z=_search( value, offset, x->data );
        if  ( z==0 ){ /*-------------------------------------------------*/ x=y; continue; }
        elif( z>=1 ){ obj->memory.push(value.slice(tmp, tmp+z)); offset=tmp +z ; return z; }
        else /*--*/ { /*--------------------------------------*/ offset=tmp;x=y; continue; }} return -1; }

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

    regex_t (): obj( new NODE() ){}

   ~regex_t () noexcept { clear_memory(); }

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

    array_t<string_t> split_view( const string_t& _str ){ ulong n = 0;
        auto idx = search_all( _str ); queue_t<string_t> out;
        if ( idx.empty()  ){ out.push(_str); return out.data(); }
        for( auto x : idx ){
             out.push( _str.slice_view( n, x[0] ) ); n = x[1];
        }    out.push( _str.slice_view( n ) ); return out.data();
    }

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
        for( auto x : idx ){
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

#else

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class regex_t : public generator_t {
protected:

    struct NODE {
        queue_t<string_t> memory;
        string_t regex, _data;
        bool icase, j=false;
        ptr_t<int> _rep;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    queue_t<int> get_next_regex( ulong _pos=0 ){ queue_t<int> out ({ 0 });

        while( _pos < obj->regex.size() ){
            if( obj->regex[_pos] == '|' ){ out.push(_pos+1); }
            if( obj->regex[_pos] == '[' ||
                obj->regex[_pos] == '{' ||
                obj->regex[_pos] == '('
             ){ _pos = get_next_key( _pos ); continue; }
            if( obj->regex[_pos] =='\\' ){ ++_pos; } ++_pos;
        }

    return out; }

    ulong get_next_key( ulong _pos ){
    uchar k=0; while( _pos < obj->regex.size() && k < 128 ){

        switch( obj->regex[_pos] ){  case '\\': ++_pos; break;
            case '[': k += 1; break; case ']' : k -= 1; break;
            case '(': k += 2; break; case ')' : k -= 2; break;
            case '{': k += 3; break; case '}' : k -= 3; break;
        }   if( k==0 ) /**/ { break; } 
            
    ++_pos; } return _pos; }

    ptr_t<int> get_rep( int pos, int npos ){
    ptr_t<int>     rep({ 0, 0 }); 
    
        bool b=0; string_t num[2];

        obj->regex.slice_view( pos+1, npos ).map([&]( char& data ){
            if  (!string::is_digit(data) ){ b =! b; }
            elif( string::is_digit(data) ){ num[b].push(data); }
        });

        if( !num[0].empty() ){ rep[0] =string::to_int(num[0]);
        /*------------------*/ rep[1] =string::to_int(num[0]); }
        if( !num[1].empty() ){ rep[1] =string::to_int(num[1]); }

        return rep;
    }

    int get_next_repeat( int pos ){

        if( obj->regex[pos] == '{' ) {
            auto npos = get_next_key ( pos );
            obj->_rep = get_rep( pos, npos ); pos = npos + 1;
        }

        elif( obj->regex[pos] == '?' ){ ++pos; obj->_rep = ptr_t<int>({ 0, 1 }); }
        elif( obj->regex[pos] == '*' ){ ++pos; obj->_rep = ptr_t<int>({ 0,-1 }); }
        elif( obj->regex[pos] == '+' ){ ++pos; obj->_rep = ptr_t<int>({ 1,-1 }); }
        else /*--------------------*/ {        obj->_rep = ptr_t<int>({ 1, 0 }); } 
        
    return pos; }

    /*─······································································─*/

    int check_reg_pattern( string_t str, ptr_t<ulong>& off, ptr_t<int>& pos ){

        goto CHCK; MORE: ++pos[1]; CHCK:

            if( obj->_data[0]!=0x01    &&
                obj->_data[0]!=0x03    &&
            (ulong) pos[1]>=str.size() ){ goto DONE; }

            if( (uchar) obj->_data[0] == '(' ){
                regex_t reg ( obj->_data.slice(1), obj->icase );
                auto idx = reg._search( str,pos[1] );
                  if ( idx == nullptr ){ goto DONE; }
                else { pos[1] = idx[1]-1; off[1] = idx[1];
                       obj->memory.push( str.slice( idx[0], idx[1] ) );
                goto LESS; }
            } elif( (uchar) obj->_data[0] == '[' ){

                auto    x = obj->_data[1] == '^' ? 2 : 1;
                auto list = compile_range(obj->_data.slice_view(x));

                  if ( x == 2 && list.none([&]( char itm ){ return comparator( str[pos[1]], itm ); }))
                     { ++off[1]; goto LESS; }
                elif ( x == 1 && list.some([&]( char itm ){ return comparator( str[pos[1]], itm ); }))
                     { ++off[1]; goto LESS; } goto DONE;

            } elif( (uchar) obj->_data[0] == 0x00 ){ goto CLSE;
            } elif( (uchar) obj->_data[0] <= 0x02 ){
                if( compile_flg( obj->_data[0], obj->j, str, pos[1] ) )
                  { goto SKIP; } goto DONE;
            } elif( (uchar) obj->_data[0] <= 0x0f ){
                if( compile_cmd( obj->_data[0], obj->j, str, pos[1] ) )
                  { ++off[1]; goto LESS; } goto DONE;
            } else {
                if( comparator( str[pos[1]], obj->_data[0] ) )
                  { ++off[1]; goto LESS; } goto DONE;
            }

        LESS: ++pos[2];

            if  ( obj->_rep[1] ==-1 )/**/{ goto MORE; }

            elif( obj->_rep[1] == 0 ){
            if  ( pos[2] < obj->_rep[0] ){ goto MORE; }} 
            
            else{
            if  ( pos[2] < obj->_rep[0] ){ goto MORE; }
            elif( pos[2] < obj->_rep[1] ){ goto MORE; }}

        DONE:

            if( (ulong) pos[1] >= str.size() ){

                if( (ulong) pos[0] <= obj->regex.last() &&
                ! ( obj->regex[pos[0]] == '$' || obj->regex[pos[0]] == '*' ||
                    obj->regex[pos[0]] == '+' || obj->regex[pos[0]] == '?' ||
                    obj->regex[pos[0]] == '|' || obj->regex[pos[0]] == '{' ||
                    obj->regex[pos[0]] == '}' || obj->regex[pos[0]] == '[' ||
                    obj->regex[pos[0]] == ']' || obj->regex[pos[0]] == '(' ||
                    obj->regex[pos[0]] == ')'
                ) ) { goto FAIL; }

                if( pos[2] >= obj->_rep[0] ){ goto CLSE; } goto FAIL;

            } elif( pos[2] == 0 && obj->_rep[0] == 0 ){
                goto SKIP;
            } elif( obj->_rep[1] == -1 ){
                if( pos[2] >=obj->_rep[0] ){ goto SKIP; } goto FAIL;
            } elif( obj->_rep[1] ==  0 ){
                if( pos[2] >=obj->_rep[0] ){ goto NEXT; } goto FAIL;
            } else { 
                if( pos[2] < obj->_rep[0] ){ goto FAIL; }
                if( pos[2] < obj->_rep[1] ){ goto SKIP; } goto NEXT;
            }

        FAIL: pos[2] = 0; return -1;
        NEXT: pos[2] = 0; return  1;
        SKIP: pos[2] = 0; return  0;
        CLSE: pos[2] = 0; return -2;

    }

    /*─······································································─*/

    bool compile_cmd( char& flg, bool b, string_t& data, int pos ){
        if  ( flg == 0x03 &&  b &&  ( pos==0||(ulong)pos>=data.size()-1) ){ return true; }
        elif( flg == 0x03 && !b && !( pos==0||(ulong)pos>=data.size()-1) ){ return true; }
        elif( flg == 0x04 &&  b &&  string::is_alnum( data[pos] ) )       { return true; }
        elif( flg == 0x04 && !b && !string::is_alnum( data[pos] ) )       { return true; }
        elif( flg == 0x05 &&  b &&  string::is_digit( data[pos] ) )       { return true; }
        elif( flg == 0x05 && !b && !string::is_digit( data[pos] ) )       { return true; }
        elif( flg == 0x06 &&  b &&  string::is_space( data[pos] ) )       { return true; }
        elif( flg == 0x06 && !b && !string::is_space( data[pos] ) )       { return true; }
        elif( flg == 0x07 &&  b &&  string::is_print( data[pos] ) )       { return true; }
        elif( flg == 0x07 && !b && !string::is_print( data[pos] ) )       { return true; }
        elif( flg == 0x0c )                                               { return true; }
        elif( flg == 0x0f )                         { return obj->_data[1] == data[pos]; }
        else{ return data[pos] == flg; } return false;
    }

    /*─······································································─*/

    bool compile_flg( char& flg, bool /**/, string_t& data, int pos ){
          if( flg == 0x01 && ((ulong)pos >= data.size()-1) ){ return true; }
        elif( flg == 0x02 &&         pos == 0 )             { return true; } return false;
    }

    /*─······································································─*/

    string_t compile_range( string_t nreg ){ queue_t<char> reg;

        for( ulong x=0; x<nreg.size(); ++x ){
            if ( nreg[x]=='\\' ){ ++x;
            if ( x < nreg.size() ){ reg.push( nreg[x] ); }
            } elif ( nreg[x+1] == '-' && (x+2)<nreg.size() ){
                auto a = min( nreg[x], nreg[x+2] );
                auto b = max( nreg[x], nreg[x+2] );
                for( auto j=a; j<=b; ++j )
                   { reg.push(j); } x+=2;
            } else { reg.push( nreg[x] ); }
        }            reg.push( '\0' );

        return reg.data();
    }

    /*─······································································─*/

    int compile( const string_t& str, ptr_t<ulong>& off, ptr_t<int>& pos ){
        if( str == nullptr || obj->regex == nullptr ){ return -1; }

        if( obj->_rep == nullptr && obj->_data != nullptr ){
            pos[0] = get_next_repeat( pos[0] );
        switch( check_reg_pattern( str, off, pos ) ){
            case -1: obj->_data=nullptr; obj->_rep=nullptr; off[0]=off[1]; return -1;
            case  1: obj->_data=nullptr; obj->_rep=nullptr; ++pos[1];      return  1;
            case  0: obj->_data=nullptr; obj->_rep=nullptr;                return  1;
            default: obj->_data=nullptr; obj->_rep=nullptr;                return -1;
        }}

    coBegin

        do { coNext;

            if( obj->regex[pos[0]] == ']' || obj->regex[pos[0]] == '{' ||
                obj->regex[pos[0]] == '}' || obj->regex[pos[0]] == ')'
            ) { off.fill(0x00); break; }

            elif( obj->regex[pos[0]] == '(' || obj->regex[pos[0]] == '[' ){
                 auto npos = get_next_key( pos[0] );
            if ( npos >= obj->regex.size() ){ off.fill(0x00); break; }
                 obj->_data = obj->regex.slice_view( pos[0], npos ); pos[0] = npos;
            }

            elif( obj->regex[pos[0]] == '|' ){ break; }
            elif( obj->regex[pos[0]] == '$' ){ obj->_data.clear(); obj->_data.push( (char) 0x01 ); }
            elif( obj->regex[pos[0]] == '^' ){ obj->_data.clear(); obj->_data.push( (char) 0x02 ); }
            elif( obj->regex[pos[0]] == '.' ){ obj->_data.clear(); obj->_data.push( (char) 0x0c ); }

            elif( obj->regex[pos[0]] == '\\' ){ ++pos[0];
                                                obj->_data.clear(); obj->_data.push( (char) 0x0f );
            if  ( obj->regex[pos[0]] == 'b'  ){ obj->_data.clear(); obj->_data.push( (char) 0x03 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'B'  ){ obj->_data.clear(); obj->_data.push( (char) 0x03 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 'w'  ){ obj->_data.clear(); obj->_data.push( (char) 0x04 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'W'  ){ obj->_data.clear(); obj->_data.push( (char) 0x04 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 'd'  ){ obj->_data.clear(); obj->_data.push( (char) 0x05 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'D'  ){ obj->_data.clear(); obj->_data.push( (char) 0x05 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 's'  ){ obj->_data.clear(); obj->_data.push( (char) 0x06 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'S'  ){ obj->_data.clear(); obj->_data.push( (char) 0x06 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 'n'  ){ obj->_data.clear(); obj->_data.push( (char) 0x07 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'N'  ){ obj->_data.clear(); obj->_data.push( (char) 0x07 ); obj->j = false; }
            else{ obj->_data += string::to_string(obj->regex[pos[0]]); }}

            else {
                obj->_data = string::to_string(obj->regex[pos[0]]);
            }   obj->_rep  = nullptr;

        } while( (ulong)pos[0] ++< obj->regex.size() );

        obj->_data=nullptr; obj->_rep=nullptr;

    coFinish }

    bool comparator( char a, char b ) const noexcept {
        return obj->icase ? ( string::to_lower(a)==string::to_lower(b) ) 
                          : ( a == b );
    }

    /*─······································································─*/

    ptr_t<ulong> _search( string_t _str, int off=0 ){
        ptr_t<ulong> res ({ (ulong)off, (ulong)off });
        auto steps = get_next_regex();

        auto y = steps.first(); while( y != nullptr ){
        auto x = y->data;

            ptr_t<int> pos ({ x, off, 0 }); res[0] = off; res[1] = off;
            while( compile( _str, res, pos )==1 ){}
               if( res[0] != res[1] ){ break; }

        y = y->next; }

        return res[0]==res[1] ? nullptr : res;
    }

public:

   ~regex_t () noexcept { clear_memory(); }

    regex_t () noexcept : obj( new NODE() ){}

    regex_t ( const string_t& reg, bool icase=false ): obj( new NODE() )
            { obj->icase = icase; obj->regex = reg; }

    /*─······································································─*/

    void clear_memory() /*------*/ const noexcept { obj->memory.clear(); }

    array_t<string_t> get_memory() const noexcept { return obj->memory.data(); }

    /*─······································································─*/

    ptr_t<ulong> search( string_t _str, uint off=0 ){
    ptr_t<ulong> out; while( off < _str.size() ){
        if(( out=_search( _str, off ) ).null() )
          { ++off; continue; } break;
    }   return out; }

    /*─······································································─*/

    array_t<ptr_t<ulong>> search_all( const string_t& _str ){
    queue_t<ptr_t<ulong>> out; ulong off=0; for(;;) {
        auto idx = search( _str, off );
        if( idx   ==nullptr ){ return out.data(); }
        if( idx[0]==idx[1]  ){ return out.data(); } off=idx[1];
            ptr_t<ulong> mem({ idx[0], idx[1] }); out.push(mem);
    }   return nullptr; }

    /*─······································································─*/

    array_t<string_t> split_view( const string_t& _str ){ ulong n = 0;
        auto idx = search_all( _str ); queue_t<string_t> out;
        if ( idx.empty()  ){ out.push(_str); return out.data(); }
        for( auto x : idx ){
             out.push( _str.slice_view( n, x[0] ) ); n = x[1];
        }    out.push( _str.slice_view( n ) ); return out.data();
    }

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
        auto idx = search( _str, off );
        if( idx == nullptr )  { return _str; }
        if( idx[0] == idx[1] ){ return _str; }
        _str.splice( idx[0], idx[1] - idx[0], _rep ); return _str;
    }

    /*─······································································─*/

    string_t remove_all( string_t _str ){
        auto idx = search_all( _str ).reverse(); for( auto x : idx ){
             _str.splice( x[0], x[1] - x[0] );
        }    return _str;
    }

    string_t remove( string_t _str, ulong off=0 ){
        auto idx = search( _str, off );
        if( idx == nullptr )  { return _str; }
        if( idx[0] == idx[1] ){ return _str; }
        _str.splice( idx[0], idx[1] - idx[0] ); return _str;
    }

    /*─······································································─*/

    array_t<string_t> match_all( const string_t& _str ){
        auto idx = search_all( _str ); queue_t<string_t> out;
        for( auto x : idx ){
             out.push(_str.slice( x[0], x[1] ));
        }    return out.data();
    }

    /*─······································································─*/

    string_t match( const string_t& _str, ulong off=0 ){
        auto idx = search( _str, off );
        if( idx == nullptr )  { return nullptr; }
        if( idx[0] == idx[1] ){ return nullptr; }
        return _str.slice( idx[0], idx[1] );
    }

    /*─······································································─*/

    bool test( const string_t& _str, ulong off=0 ){
        auto idx = search( _str, off );
        if( idx == nullptr )  { return 0; }
        if( idx[0] == idx[1] ){ return 0; }
                                return 1;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace regex {

    inline string_t remove( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.remove( _str );
    }

    /*─······································································─*/

    inline string_t replace_all( const string_t& _str, const string_t& _reg, const string_t& _rep, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.replace_all( _str, _rep );
    }

    /*─······································································─*/

    inline string_t remove_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.remove_all( _str );
    }

    /*─······································································─*/

    inline array_t<ptr_t<ulong>> search_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.search_all( _str );
    }

    /*─······································································─*/

    inline string_t replace( const string_t& _str, const string_t& _reg, const string_t& _rep, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.replace( _str, _rep );
    }

    /*─······································································─*/

    inline array_t<string_t> get_memory( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); reg.search_all( _str ); return reg.get_memory();
    }

    /*─······································································─*/

    inline array_t<string_t> match_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.match_all( _str );
    }

    /*─······································································─*/

    inline ptr_t<ulong> search( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.search( _str );
    }

    /*─······································································─*/

    inline string_t match( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.match( _str );
    }

    /*─······································································─*/

    inline bool test( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.test( _str );
    }

    /*─······································································─*/

    inline array_t<string_t> split_view( const string_t& _str, char ch ){ 
        return string::split_view( _str, ch ); }

    inline array_t<string_t> split_view( const string_t& _str, int  ch ){ 
        return string::split_view( _str, ch ); }

    inline array_t<string_t> split_view( const string_t& _str, const string_t& _reg, bool _flg=false ){
          if ( _reg.size ()== 1 ){ return string::split_view( _str, _reg[0] ); }
        elif ( _reg.empty() )    { return string::split_view( _str, 1 ); }
        regex_t reg( _reg, _flg ); return reg.split_view( _str );
    }

    /*─······································································─*/

    inline array_t<string_t> split( const string_t& _str, char ch ){ return string::split( _str, ch ); }

    inline array_t<string_t> split( const string_t& _str, int ch ){ return string::split( _str, ch ); }

    inline array_t<string_t> split( const string_t& _str, const string_t& _reg, bool _flg=false ){
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
        thread_local static ptr_t<regex_t> reg ({
               regex_t( "\\$\\{\\d+\\}" ),
               regex_t( "\\d+" )
        });

        for( auto &x: reg[0].search_all( val ) ){
        auto y = string::to_uint( reg[1].match( val.slice_view( x[0], x[1] ) ) );
        if ( y >= count ){ continue; }
             out.push( val.slice_view( idx,x[0] ) );
             out.push( string::get( y , args... ) ); idx = x[1];
        }    out.push( val.slice_view( idx ) );

        return array_t<string_t>( out.data() ).join("");
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/