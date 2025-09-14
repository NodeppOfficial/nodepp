
/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_OBJECT
#define NODEPP_OBJECT

/*────────────────────────────────────────────────────────────────────────────*/

#include "any.h"
#include "map.h"
#include "initializer.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace type { 

    template< class T > struct obj_type_id                    { static constexpr int value = 0x0000 |  0; };

    template<>          struct obj_type_id<int>               { static constexpr int value = 0x0000 |  1; };
    template<>          struct obj_type_id<uint>              { static constexpr int value = 0x0000 |  2; };
    template<>          struct obj_type_id<bool>              { static constexpr int value = 0x0000 |  3; };
    template<>          struct obj_type_id<char>              { static constexpr int value = 0x0000 |  4; };
    template<>          struct obj_type_id<long>              { static constexpr int value = 0x0000 |  5; };
    template<>          struct obj_type_id<short>             { static constexpr int value = 0x0000 |  6; };
    template<>          struct obj_type_id<uchar>             { static constexpr int value = 0x0000 |  7; };
    template<>          struct obj_type_id<llong>             { static constexpr int value = 0x0000 |  8; };
    template<>          struct obj_type_id<ulong>             { static constexpr int value = 0x0000 |  9; };
    template<>          struct obj_type_id<ushort>            { static constexpr int value = 0x0000 | 10; };
    template<>          struct obj_type_id<ullong>            { static constexpr int value = 0x0000 | 11; };

    template<>          struct obj_type_id<wchar_t>           { static constexpr int value = 0x0000 | 12; };
    template<>          struct obj_type_id<char16_t>          { static constexpr int value = 0x0000 | 13; };
    template<>          struct obj_type_id<char32_t>          { static constexpr int value = 0x0000 | 14; };

    template<>          struct obj_type_id<float>             { static constexpr int value = 0x0000 | 15; };
    template<>          struct obj_type_id<double>            { static constexpr int value = 0x0000 | 16; };
    template<>          struct obj_type_id<ldouble>           { static constexpr int value = 0x0000 | 17; };

    template<>          struct obj_type_id<string_t>          { static constexpr int value = 0x0000 | 18; };
    template< ulong N > struct obj_type_id<char[N]>           { static constexpr int value = 0x0000 | 18; };
    template<>          struct obj_type_id<char*>             { static constexpr int value = 0x0000 | 18; };

    template< class T >          struct obj_type_id<T*>       { static constexpr int value = 0xf000 | obj_type_id<T>::value; };
    template< class T, ulong N > struct obj_type_id<T[N]>     { static constexpr int value = 0xf200 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<initializer_t<T>>  { static constexpr int value = 0xf400 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<ptr_t<T>>          { static constexpr int value = 0xf600 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<queue_t<T>>        { static constexpr int value = 0xf800 | obj_type_id<T>::value; };

    template< class T > struct obj_type_id<array_t<T>>        { static constexpr int value = 0xfA00 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<map_t<string_t,T>> { static constexpr int value = 0xfC00 | obj_type_id<T>::value; };

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class object_t {
private:

    using T     = type::pair<string_t,object_t>;
    using QUEUE = map_t<string_t,object_t>;
    using ARRAY = array_t<object_t>;

protected:

    struct NODE {
        any_t  mem     ;
        int   type  =-1;
    };  ptr_t<NODE> obj;

public:

    template< ulong N >
    object_t( const T (&arr) [N] ) : obj(new NODE()) {
        QUEUE mem; for( ulong x=0; x<N; ++x )
            { mem[arr[x].first]= arr[x].second; }
        obj->mem = mem; obj->type = 20;
    }

    object_t( null_t ) : obj( new NODE() ) { /*---*/ }

    template< class U >
    object_t( const U& any ) : obj(new NODE()) {
        if( type::is_same<U,ARRAY>::value )
          { obj->type = 21; goto BACK; }
      elif( type::is_same<U,QUEUE>::value )
          { obj->type = 20; goto BACK; }
        obj->type = type::obj_type_id<U>::value;
        BACK:; obj->mem = any;
    }

    object_t() : obj( new NODE() ) {}

    virtual ~object_t() noexcept {}

    /*─······································································─*/

    template< class U > bool is() const {
        if( get_type_id()==21 && type::is_same<U,ARRAY>::value ){ return true; }
      elif( get_type_id()==20 && type::is_same<U,QUEUE>::value ){ return true; }
      elif( get_type_id()     == type::obj_type_id<U>::value   ){ return true; } return false;
    }

    template< class U >
    explicit operator U() const { return /*-------------*/ obj->mem.as<U>    (); }
    bool has_value()      const { return obj->type<0?false:obj->mem.has_value(); }
    uint type_size()      const { return obj->type<0?false:obj->mem.type_size(); }

    template< class U > U as() const { 
    try { return obj->mem.as<U>(); } catch( except_t ) {
          throw except_t( "item does not exists" );
    } }

    /*─······································································─*/

    object_t& operator[]( const string_t& name ) const {
        if( obj->type != 20 ){ 
            QUEUE mem; mem[name] = object_t();
            obj->mem = mem; obj->type=20; /**/
        }   return obj->mem.as<QUEUE>()[name];
    }

    object_t& operator[]( const ulong& idx ) const {
        if( !has_value() ){ throw except_t("item is empty"); }
        if( !is<ARRAY>() ){ throw except_t("item isn't an array"); }
        return obj->mem.as<ARRAY>()[idx];
    }

    /*─······································································─*/

    bool has( const ulong& idx ) const {
        if( !has_value() || obj->type!=21 ){ return false; }
        return obj->mem.as<ARRAY>().size()>=idx;
    }

    bool has( const string_t& name ) const {
        if( !has_value() || obj->type != 20  )
          { obj->mem=QUEUE(); obj->type= 20; }
        return obj->mem.as<QUEUE>().has(name);
    }

    /*─······································································─*/

    array_t<string_t> keys() const { queue_t<string_t> res;
        if( obj->type == 20 ){ auto mem = obj->mem.as<QUEUE>();
            mem.map([&]( T item ){ res.push( item.first ); });
        }   return res.data();
    }

    int get_type_id() const { return obj->type; }

    /*─······································································─*/

    bool empty() const {
        if( obj->type == 21 )
          { return obj->mem.as<ARRAY>().empty(); }
        if( obj->type == 20 )
          { return obj->mem.as<QUEUE>().empty(); }
        return obj->mem.empty();
    }

    ulong size() const {
        if( obj->type == 21 )
          { return obj->mem.as<ARRAY>().size(); }
        if( obj->type == 20 )
          { return obj->mem.as<QUEUE>().size(); }
        return 0;
    }

    /*─······································································─*/

    void erase( const string_t& name ) const {
        if( !has_value() || obj->type != 20  )
          { obj->mem=QUEUE(); obj->type= 20; }
            obj->mem.as<QUEUE>().erase(name);
    }

    void erase() { obj = new NODE(); }

    void clear() { obj = new NODE(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
