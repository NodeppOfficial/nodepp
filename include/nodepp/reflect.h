/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_REFLECT
#define NODEPP_REFLECT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class reflect_t {
protected:

    using T = type::pair< string_t, void* >;

    struct NODE {
        queue_t<T> fields;
    };  ptr_t<NODE> obj;

public:

    /*─······································································─*/

    reflect_t () noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    template < typename V >
    void reflect_field( const string_t& name, V& value ) const noexcept {
         obj->fields.push({ name, type::cast<void>( &value ) });
    }

    /*─······································································─*/

    array_t<string_t> keys() const noexcept {
        array_t<string_t> res ( obj->fields.size() );
        ulong n=0; obj->fields.map([&]( T& data ){
            res[n] = data.first; n++;
        }); return res;
    }

    /*─······································································─*/

    template< class U, class V >
    void set_value( const string_t& fieldName, const V& value ) const {
        auto x = obj->fields.first(); while( x != nullptr ) {
        auto y = x->next;            
            if( x->data.first == fieldName ){
                auto data = type::cast<U>( x->data.second );
                    *data = (U)(value); return;
            }        x = y;
        }   throw except_t( "Field not found [",fieldName,"]" );
    }

    /*─······································································─*/

    template < class V, class = typename type::enable_if<!type::is_pointer<V>::value,V>::type >
    V& get_field( const string_t& fieldName ) const {
        auto x = obj->fields.first(); while( x != nullptr ) {
        auto y = x->next;      
            if( x->data.first == fieldName ){
                 return *type::cast<V>( x->data.second );
            }    x = y;
        }   throw except_t( "Field not found [",fieldName,"]" );
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif