/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EXPECTED
#define NODEPP_EXPECTED

#include "any.h"

namespace nodepp {
template <typename T, typename E> struct expected_t { 
protected:

    struct NODE { any_t data; bool has; }; ptr_t<NODE> obj;

public:

    expected_t( const T& val ) noexcept : obj( new NODE() ) { obj->has = true ; obj->data = val; }

    expected_t( const E& err ) noexcept : obj( new NODE() ) { obj->has = false; obj->data = err; }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return has_value(); }
    bool has_value() /*-------*/ const noexcept { return obj->has; }

    /*─······································································─*/

    T value() const { if( !has_value() || !obj->data.has_value() ) {
        throw  except_t("expected does not have a value");
    }   return obj->data.template as<T>(); }

    /*─······································································─*/

    E error() const { if( has_value() || !obj->data.has_value() ) {
        throw  except_t("expected does not have a value");
    }   return obj->data.template as<E>(); }

};}

#endif