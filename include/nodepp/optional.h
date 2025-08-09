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

    bool has; any_t data;

public:

    optional_t( const T& val ) noexcept { has = true; data = val; }

    optional_t() noexcept { has = false; }

    virtual ~optional_t() noexcept {}

    /*─······································································─*/

    bool has_value() const noexcept { return has; }

    /*─······································································─*/

    T value() const { if ( !has_value() || !data.has_value() ) {
        throw  except_t("Optional does not have a value");
    }   return data.as<T>(); }
    
};}

#endif