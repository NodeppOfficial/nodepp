/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_OPTIONAL
#define NODEPP_OPTIONAL

#include "any.h"

namespace nodepp { 
template< class T > class optional_t {
protected:

    struct NODE { bool has; any_t data; }; ptr_t<NODE> obj;

public:

    optional_t( const T& val ) noexcept : obj( new NODE() ) { obj->has = true ; obj->data = val; }

    optional_t()  /*--------*/ noexcept : obj( new NODE() ) { obj->has = false; }

    optional_t( null_t ) /*-*/ noexcept : obj( new NODE() ) { obj->has = false; }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return has_value(); }
    bool has_value() /*-------*/ const noexcept { return obj->has; }

    /*─······································································─*/

    T value() const { if ( !has_value() || !obj->data.has_value() ) {
        throw  except_t("Optional does not have a value");
    }   return obj->data.template as<T>(); }
    
};}

#endif