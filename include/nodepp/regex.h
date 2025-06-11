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

namespace nodepp { class regex_t {
protected:

    struct NODE {
        queue_t<string_t> memory;
        string_t regex, _data;
        ptr_t<int> _rep;
        bool i, j=false;
    };  ptr_t<NODE> obj;

    /*─······································································─*/

    array_t<int> get_next_regex( ulong _pos=0 ) const {
        queue_t<int> result ({ 0 });
        while( _pos < obj->regex.size() ){
            if( obj->regex[_pos] == '|' ){ result.push(_pos+1); }
            if( obj->regex[_pos] == '[' ||
                obj->regex[_pos] == '{' ||
                obj->regex[_pos] == '('
             ){ _pos = get_next_key( _pos ); continue; }
            if( obj->regex[_pos] == '\\' ){ _pos++; } _pos++;
        }   return result.data();
    }

    int get_next_key( ulong _pos ) const {
        uchar k=0; while( _pos < obj->regex.size() ){

            switch( obj->regex[_pos] ){  case '\\': _pos++;break;
                case '[': k += 1; break; case ']': k -= 1; break;
                case '(': k += 2; break; case ')': k -= 2; break;
                case '{': k += 3; break; case '}': k -= 3; break;
            }

            if( k == 0 ){ break; } _pos++;
        }   return _pos == obj->regex.size() ? -1 : _pos;
    }

    ptr_t<int> get_rep( int pos, int npos ) const {
        ptr_t<int> rep ({ 0, 0 }); bool b=0; string_t num[2];

        obj->regex.slice( pos+1, npos ).map([&]( char& data ){
              if(!string::is_digit(data) ){ b =! b; }
            elif( string::is_digit(data) ){ num[b].push(data); }
        });

        if( !num[0].empty() ) rep[0] = string::to_int( num[0] );
        if( !num[1].empty() ) rep[1] = string::to_int( num[1] );

        return rep;
    }

    int get_next_repeat( int pos ) const {
        if( obj->regex[pos] == '{' ){
            auto npos = get_next_key( pos );
                 obj->_rep = get_rep( pos, npos ); pos = npos + 1;
        }
        elif( obj->regex[pos] == '?' ){ obj->_rep = ptr_t<int>({ 0, 1 }); pos++; }
        elif( obj->regex[pos] == '*' ){ obj->_rep = ptr_t<int>({ 0,-1 }); pos++; }
        elif( obj->regex[pos] == '+' ){ obj->_rep = ptr_t<int>({ 1,-1 }); pos++; }
        else                          { obj->_rep = ptr_t<int>({ 1, 0 });        } return pos;
    }

    /*─······································································─*/

    int check_reg_pattern( string_t str, ptr_t<ulong>& off, ptr_t<int>& pos ) const {

        goto CHCK; MORE: pos[1]++;

        CHCK:

            auto _str_ = obj->i ? string::to_lower(str[pos[1]]) : str[pos[1]];

            if( obj->_data[0]!=0x01     &&
                obj->_data[0]!=0x03     &&
                (ulong) pos[1]>=str.size()
            ) { goto DONE; }

            if( (uchar) obj->_data[0] == '(' ){
                regex_t reg ( obj->_data.slice(1), obj->i );
                auto idx = reg._search( str,pos[1] );
                  if ( idx == nullptr ){ goto DONE; }
                else { pos[1]+= idx[1]-idx[0]-1;
                       off[1]+= idx[1]-idx[0];
                       store_mem( str, idx );
                       goto LESS;
                     }
            } elif( (uchar) obj->_data[0] == '[' ) {

                auto    x = obj->_data[1] == '^' ? 2 : 1;
                auto list = compile_range(obj->_data.slice(x));

                  if ( x == 2 && list.none([&]( char itm ){ return _str_ == itm; }))
                     { off[1]++; goto LESS; }
                elif ( x == 1 && list.some([&]( char itm ){ return _str_ == itm; }))
                     { off[1]++; goto LESS; } goto DONE;

            } elif( (uchar) obj->_data[0] == 0x00 ) {
                                 goto CLSE;
            } elif( (uchar) obj->_data[0] <= 0x02 ) {
                if( compile_flg( obj->_data[0], obj->j, str, pos[1] ) )
                  { goto SKIP; } goto DONE;
            } elif( (uchar) obj->_data[0] <= 0x0f ) {
                if( compile_cmd( obj->_data[0], obj->j, str, pos[1] ) )
                  { off[1]++; goto LESS; }
                              goto DONE;
            } else {
                if( obj->_data[0] == _str_ )
                  { off[1]++; goto LESS; }
                              goto DONE;
            }

        LESS: pos[2]++;

              if( obj->_rep[1] ==-1 ){ goto MORE; }
            elif( obj->_rep[1] == 0 ){
                  if( pos[2] < obj->_rep[0] ){ goto MORE; }
                                               goto DONE;
            } else {
                  if( pos[2] < obj->_rep[0] ){ goto MORE; }
                elif( pos[2] < obj->_rep[1] ){ goto MORE; }
                else                         { goto DONE; }
            }

        DONE:

            if( (ulong) pos[1] >= str.size() ){
            if( (ulong) pos[0] <= obj->regex.last() &&
                 !( obj->regex[pos[0]] == '$'   ||
                    obj->regex[pos[0]] == '*'   ||
                    obj->regex[pos[0]] == '+'   ||
                    obj->regex[pos[0]] == '?'   ||
                    obj->regex[pos[0]] == '|'   ||
                    obj->regex[pos[0]] == '{'   ||
                    obj->regex[pos[0]] == '}'   ||
                    obj->regex[pos[0]] == '['   ||
                    obj->regex[pos[0]] == ']'   ||
                    obj->regex[pos[0]] == '('   ||
                    obj->regex[pos[0]] == ')'
                )){ goto FAIL; }
                if( pos[2] >= obj->_rep[0] ){ goto CLSE; } goto FAIL;
            } elif( pos[2] == 0 && obj->_rep[0] == 0 ){
                goto SKIP;
            } elif( obj->_rep[1] ==-1 ){
                if( pos[2] >=obj->_rep[0] ){ goto SKIP; } goto FAIL;
            } elif( obj->_rep[1] != 0 ) {
                if( pos[2] > obj->_rep[1] ){ goto NEXT; }
              elif( pos[2] >=obj->_rep[0] ){ goto NEXT; } goto FAIL;
            } else {
                if( pos[2] >=obj->_rep[0] ){ goto NEXT; } goto FAIL;
            }

        CLSE: pos[2] = 0; return -2;
        FAIL: pos[2] = 0; return -1;
        NEXT: pos[2] = 0; return  1;
        SKIP: pos[2] = 0; return  0;

    }

    /*─······································································─*/

    bool compile_cmd( char& flg, bool b, string_t& data, int pos ) const {
          if( flg == 0x03 &&  b &&  ( pos==0||(ulong)pos>=data.size()-1) ){ return true; }
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

    bool compile_flg( char& flg, bool /**/, string_t& data, int pos ) const {
          if( flg == 0x01 && ((ulong)pos >= data.size()-1) ){ return true; }
        elif( flg == 0x02 &&         pos == 0 )             { return true; } return false;
    }

    /*─······································································─*/

    string_t compile_range( string_t nreg ) const {
        string_t reg;

        for( ulong x=0; x<nreg.size(); x++ ){
            if ( nreg[x]=='\\' ){ x++;
            if ( x < nreg.size() ){ reg.push( nreg[x] ); }
            } elif ( nreg[x+1] == '-' && (x+2)<nreg.size() ){
                auto a = min( nreg[x], nreg[x+2] );
                auto b = max( nreg[x], nreg[x+2] );
                for( auto j=a; j<=b; j++ )
                   { reg.push(j); } x+=2;
            } else { reg.push( nreg[x] ); }
        }

        return reg;
    }

    /*─······································································─*/

    void store_mem( string_t& data, ptr_t<ulong>& pos ) const {
        if( pos == nullptr ){ return; }
        obj->memory.push( data.slice( pos[0], pos[1] ) );
    }

    /*─······································································─*/

    int compile( const string_t& str, ptr_t<ulong>& off, ptr_t<int>& pos ) const {
        if( str == nullptr || obj->regex == nullptr ){ return -1; }

        if( obj->_rep == nullptr && obj->_data != nullptr ){
            pos[0] = get_next_repeat( pos[0] );
        switch( check_reg_pattern( str, off, pos ) ){
            case -1: obj->_data=nullptr; obj->_rep=nullptr; off[0]=off[1]; return -1;
            case  1: obj->_data=nullptr; obj->_rep=nullptr; pos[1]++;      return  1;
            case  0: obj->_data=nullptr; obj->_rep=nullptr;                return  1;
            default: obj->_data=nullptr; obj->_rep=nullptr;                return -1;
        }}

    coStart

        do { coNext;

              if( obj->regex[pos[0]] == ']' || obj->regex[pos[0]] == '{' ||
                  obj->regex[pos[0]] == '}' || obj->regex[pos[0]] == ')'
               ){ _ERROR(string::format( "regex: %d %c",pos[0], obj->regex[pos[0]] )); }

            elif( obj->regex[pos[0]] == '(' || obj->regex[pos[0]] == '[' ){
                 auto npos = get_next_key( pos[0] );
            if ( npos < 0 )
               { _ERROR(string::format( "regex: %d %c", pos[0], obj->regex[pos[0]] )); }
                 obj->_data = obj->regex.slice( pos[0], npos ); pos[0] = npos;
            }

            elif( obj->regex[pos[0]] == '|' ){ break; }
            elif( obj->regex[pos[0]] == '$' ){ obj->_data.clear(); obj->_data.push( (char) 0x01 ); }
            elif( obj->regex[pos[0]] == '^' ){ obj->_data.clear(); obj->_data.push( (char) 0x02 ); }
            elif( obj->regex[pos[0]] == '.' ){ obj->_data.clear(); obj->_data.push( (char) 0x0c ); }

            elif( obj->regex[pos[0]] == '\\' ){ pos[0]++;
                                                obj->_data.clear(); obj->_data.push( (char) 0x0f );
              if( obj->regex[pos[0]] == 'b'  ){ obj->_data.clear(); obj->_data.push( (char) 0x03 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'B'  ){ obj->_data.clear(); obj->_data.push( (char) 0x03 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 'w'  ){ obj->_data.clear(); obj->_data.push( (char) 0x04 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'W'  ){ obj->_data.clear(); obj->_data.push( (char) 0x04 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 'd'  ){ obj->_data.clear(); obj->_data.push( (char) 0x05 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'D'  ){ obj->_data.clear(); obj->_data.push( (char) 0x05 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 's'  ){ obj->_data.clear(); obj->_data.push( (char) 0x06 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'S'  ){ obj->_data.clear(); obj->_data.push( (char) 0x06 ); obj->j = false; }
            elif( obj->regex[pos[0]] == 'n'  ){ obj->_data.clear(); obj->_data.push( (char) 0x07 ); obj->j = true;  }
            elif( obj->regex[pos[0]] == 'N'  ){ obj->_data.clear(); obj->_data.push( (char) 0x07 ); obj->j = false; }
            else{ obj->_data += string::to_string(obj->regex[pos[0]]); }
            }

            else {
                obj->_data = string::to_string(obj->regex[pos[0]]);
            }   obj->_rep  = nullptr;

        } while( (ulong)++pos[0]<=obj->regex.size() );

        obj->_data=nullptr; obj->_rep=nullptr;

    coStop
    }

public: regex_t (): obj( new NODE() ) {}

    regex_t ( const string_t& reg, bool icase=false ): obj( new NODE() )
            { obj->i = icase; obj->regex = reg; }

    /*─······································································─*/

    ptr_t<string_t> get_memory() const {
        return obj->memory.data();
    }

    /*─······································································─*/

    ptr_t<ulong> _search( string_t _str, int off=0 ) const {
        ptr_t<ulong> res ({ (ulong)off, (ulong)off });

        for( auto &x: get_next_regex() ){
             ptr_t<int> pos ({ x, off, 0 }); res[0] = off; res[1] = off;
             while( compile( _str, res, pos )==1 );
                if( res[0] != res[1] ){ break; }
        }

        return res[0]==res[1] ? nullptr : res;
    }

    /*─······································································─*/

    ptr_t<ulong> search( string_t _str, uint off=0 ) const {
        ptr_t<ulong> result; while( off < _str.size() ){
            result = _search( _str, off );
            if( result != nullptr )
              { break; } off++;
        }   return result;
    }

    /*─······································································─*/

    array_t<ptr_t<ulong>> search_all( const string_t& _str ) const {
        queue_t<ptr_t<ulong>> result; ulong off=0; do {
            auto idx = search( _str, off );
            if( idx    == nullptr ){ return result.data(); }
            if( idx[0] == idx[1]  ){ return result.data(); } off=idx[1];
                ptr_t<ulong> mem({ idx[0], idx[1] }); result.push(mem);
        } while(1); return nullptr;
    }

    /*─······································································─*/

    array_t<string_t> split( const string_t& _str ) const { ulong n = 0;
        auto idx = search_all( _str ); queue_t<string_t> result;
        if ( idx.empty()  ){ return result.data(); }
        for( auto x : idx ){
            result.push( _str.slice( n, x[0] ) ); n = x[1];
        }   result.push( _str.slice( n ) ); return result.data();
    }

    /*─······································································─*/

    string_t replace_all( string_t _str, const string_t& _rep ) const {
        auto idx = search_all( _str ).reverse(); for( auto x : idx ){
            _str.splice( x[0], x[1] - x[0], _rep );
        }   return _str;
    }

    string_t replace( string_t _str, const string_t& _rep, ulong off=0 ) const {
        auto idx = search( _str, off );
        if( idx == nullptr )  { return _str; }
        if( idx[0] == idx[1] ){ return _str; }
            _str.splice( idx[0], idx[1] - idx[0], _rep ); return _str;
    }

    /*─······································································─*/

    string_t remove_all( string_t _str ) const {
        auto idx = search_all( _str ).reverse(); for( auto x : idx ){
            _str.splice( x[0], x[1] - x[0] );
        }   return _str;
    }

    string_t remove( string_t _str, ulong off=0 ) const {
        auto idx = search( _str, off );
        if( idx == nullptr )  { return _str; }
        if( idx[0] == idx[1] ){ return _str; }
            _str.splice( idx[0], idx[1] - idx[0] ); return _str;
    }

    /*─······································································─*/

    array_t<string_t> match_all( const string_t& _str ) const {
        auto idx = search_all( _str ); queue_t<string_t> result;
        for( auto x : idx ){
            result.push(_str.slice( x[0], x[1] ));
        }   return result.data();
    }

    /*─······································································─*/

    string_t match( const string_t& _str, ulong off=0 ) const {
        auto idx = search( _str, off );
        if( idx == nullptr )  { return nullptr; }
        if( idx[0] == idx[1] ){ return nullptr; }
            return _str.slice( idx[0], idx[1] );
    }

    /*─······································································─*/

    bool test( const string_t& _str, ulong off=0 ) const {
        auto idx = search( _str, off );
        if( idx == nullptr )  { return 0; }
        if( idx[0] == idx[1] ){ return 0; }
                                return 1;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace regex {

    string_t replace_all( const string_t& _str, const string_t& _reg, const string_t& _rep, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.replace_all( _str, _rep );
    }

    string_t replace_all( const string_t& _str, const regex_t& reg, const string_t& _rep ){
        return reg.replace_all( _str, _rep );
    }

    /*─······································································─*/

    string_t remove_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.remove_all( _str );
    }

    string_t remove_all( const string_t& _str, const regex_t& reg ){
        return reg.remove_all( _str );
    }

    /*─······································································─*/

    array_t<ptr_t<ulong>> search_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.search_all( _str );
    }

    array_t<ptr_t<ulong>> search_all( const string_t& _str, const regex_t& reg ){
        return reg.search_all( _str );
    }

    /*─······································································─*/

    string_t replace( const string_t& _str, const string_t& _reg, const string_t& _rep, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.replace( _str, _rep );
    }

    string_t replace( const string_t& _str, const regex_t& reg, const string_t& _rep ){
        return reg.replace( _str, _rep );
    }

    /*─······································································─*/

    string_t remove( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.remove( _str );
    }

    string_t remove( const string_t& _str, const regex_t& reg ){
        return reg.remove( _str );
    }

    /*─······································································─*/

    array_t<string_t> get_memory( const string_t& _str, const regex_t& reg, bool _flg=false ){
        reg.search_all( _str ); return reg.get_memory();
    }

    array_t<string_t> get_memory( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); reg.search_all( _str ); return reg.get_memory();
    }

    /*─······································································─*/

    array_t<string_t> match_all( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.match_all( _str );
    }

    array_t<string_t> match_all( const string_t& _str, const regex_t& reg ){
        return reg.match_all( _str );
    }

    /*─······································································─*/

    ptr_t<ulong> search( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.search( _str );
    }

    ptr_t<ulong> search( const string_t& _str, const regex_t& reg ){
        return reg.search( _str );
    }

    /*─······································································─*/

    string_t match( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.match( _str );
    }

    string_t match( const string_t& _str, const regex_t& reg ){
        return reg.match( _str );
    }

    /*─······································································─*/

    bool test( const string_t& _str, const string_t& _reg, bool _flg=false ){
        regex_t reg( _reg, _flg ); return reg.test( _str );
    }

    bool test( const string_t& _str, const regex_t& reg ){
        return reg.test( _str );
    }

    /*─······································································─*/

    array_t<string_t> split( const string_t& _str, char ch ){ return string::split( _str, ch ); }

    array_t<string_t> split( const string_t& _str, int ch ){ return string::split( _str, ch ); }

    array_t<string_t> split( const string_t& _str, const string_t& _reg, bool _flg=false ){
          if ( _reg.size() == 1 ){  return string::split( _str, _reg[0] ); }
        elif ( _reg.empty() ) { return string::split( _str, 1 ); }
        regex_t reg(_reg,_flg); return reg.split( _str );
    }

    array_t<string_t> split( const string_t& _str, const regex_t& reg ){
        return reg.split( _str );
    }

    /*─······································································─*/

    template< class T, class... V > string_t join( const string_t& c, const T& argc, const V&... args ){
        return string::join( c, argc, args... );
    }

    /*─······································································─*/

    template< class V, class... T > string_t format( const V& val, const T&... args ){
        string_t result = string::to_string(val); ulong n=0; string::map([&]( string_t arg ){
            string_t reg = "\\$\\{" + string::to_string(n) + "\\}";
            result = replace_all(result,reg,arg); n++;
        },  args... ); return result;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif