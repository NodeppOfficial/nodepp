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

    bool has; any_t data;

public:

    expected_t( const T& val ) { has = true; data = val; }

    expected_t( const E& err ) { has = false;data = err; }

    virtual ~expected_t() noexcept {}

    /*─······································································─*/

    bool has_value() const noexcept { return has; }

    /*─······································································─*/

    T value() const { if ( !has_value() || !data.has_value() ) {
        throw  except_t("expected does not have a value");
    }   return data.as<T>(); }

    /*─······································································─*/

    E error() const { if ( has_value() || !data.has_value() ) {
        throw  except_t("expected does not have a value");
    }   return data.as<E>(); }

};}

#endif