
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

    template< class T > struct obj_type_id                    { static constexpr uchar_16 value = 0x0000 | 0x00; };

    template<>          struct obj_type_id<int>               { static constexpr uchar_16 value = 0x0000 | 0x01; };
    template<>          struct obj_type_id<uint>              { static constexpr uchar_16 value = 0x0000 | 0x02; };
    template<>          struct obj_type_id<bool>              { static constexpr uchar_16 value = 0x0000 | 0x03; };
    template<>          struct obj_type_id<char>              { static constexpr uchar_16 value = 0x0000 | 0x04; };
    template<>          struct obj_type_id<long>              { static constexpr uchar_16 value = 0x0000 | 0x05; };
    template<>          struct obj_type_id<short>             { static constexpr uchar_16 value = 0x0000 | 0x06; };
    template<>          struct obj_type_id<uchar>             { static constexpr uchar_16 value = 0x0000 | 0x07; };
    template<>          struct obj_type_id<llong>             { static constexpr uchar_16 value = 0x0000 | 0x08; };
    template<>          struct obj_type_id<ulong>             { static constexpr uchar_16 value = 0x0000 | 0x09; };
    template<>          struct obj_type_id<ushort>            { static constexpr uchar_16 value = 0x0000 | 0x0A; };
    template<>          struct obj_type_id<ullong>            { static constexpr uchar_16 value = 0x0000 | 0x0B; };

    template<>          struct obj_type_id<wchar_t>           { static constexpr uchar_16 value = 0x0000 | 0x0C; };
    template<>          struct obj_type_id<float>             { static constexpr uchar_16 value = 0x0000 | 0x0D; };
    template<>          struct obj_type_id<double>            { static constexpr uchar_16 value = 0x0000 | 0x0E; };
    template<>          struct obj_type_id<ldouble>           { static constexpr uchar_16 value = 0x0000 | 0x0F; };

    template<>          struct obj_type_id<string_t>          { static constexpr uchar_16 value = 0x0000 | 0x10; };
    template< ulong N > struct obj_type_id<char[N]>           { static constexpr uchar_16 value = 0x0000 | 0x10; };
    template<>          struct obj_type_id<char*>             { static constexpr uchar_16 value = 0x0000 | 0x10; };

    template< class T >          struct obj_type_id<T*>       { static constexpr uchar_16 value = 0xF000 | obj_type_id<T>::value; };
    template< class T, ulong N > struct obj_type_id<T[N]>     { static constexpr uchar_16 value = 0xF200 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<initializer_t<T>>  { static constexpr uchar_16 value = 0xF400 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<ptr_t<T>>          { static constexpr uchar_16 value = 0xF600 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<queue_t<T>>        { static constexpr uchar_16 value = 0xF800 | obj_type_id<T>::value; };

    template< class T > struct obj_type_id<array_t<T>>        { static constexpr uchar_16 value = 0xFA00 | obj_type_id<T>::value; };
    template< class T > struct obj_type_id<map_t<string_t,T>> { static constexpr uchar_16 value = 0xFC00 | obj_type_id<T>::value; };

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class object_t {
private:

    using T     = type::pair<string_t,object_t>;
    using QUEUE = map_t<string_t,object_t>;
    using ARRAY = array_t<object_t>;

protected:

    struct NODE {
        uchar_16 type = -1; any_t mem;
    };  ptr_t<NODE> obj;

public:

    object_t( null_t ) : obj( new NODE() ){}
    object_t()         : obj( new NODE() ){}

    template< ulong N >
    object_t( const T (&arr) [N] ) : obj( new NODE() ) {
        QUEUE  mem; for( ulong x=0; x<N; ++x ) { 
               mem[arr[x].first]= arr[x].second; 
        } obj->mem = mem; obj->type = 20;
    }

    template< class U >
    object_t( const U& value ) : obj( new NODE() ) { do {
        if  ( type::is_same<U,ARRAY>::value )
            { obj->type = 21; break; }
        elif( type::is_same<U,QUEUE>::value )
            { obj->type = 20; break; }
        obj->type = type::obj_type_id<U>::value;
    } while(0); obj->mem = value; }

    /*─······································································─*/

    template< class U > bool is() const {
        if  ( get_type_id()==21 && type::is_same<U,ARRAY>::value ){ return true; }
        elif( get_type_id()==20 && type::is_same<U,QUEUE>::value ){ return true; }
        elif( get_type_id()     == type::obj_type_id<U>  ::value ){ return true; } 
    return false; }

    template< class U > U as() const { return obj->mem.as<U>(); }

    template< class U >
    explicit operator    U() const { return obj->mem.as<U>(); }
    explicit operator bool() const { return has_value(); /**/ }

    bool has_value() const { return obj->type<0?false:obj->mem.has_value(); }
    uint type_size() const { return obj->type<0?false:obj->mem.type_size(); }


    /*─······································································─*/

    object_t& operator[]( const string_t& name ) const {
        if( obj->type != 20 ){ 
            QUEUE mem; mem[name] = object_t();
            obj->mem = mem; obj->type=20; /**/
        }   return obj->mem.as<QUEUE>()[name];
    }

    object_t& operator[]( const ulong& idx ) const {
        if( !has_value() ){ NODEPP_THROW_ERROR("item is empty"); }
        if( !is<ARRAY>() ){ NODEPP_THROW_ERROR("item isn't an array"); }
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

/*────────────────────────────────────────────────────────────────────────────*/