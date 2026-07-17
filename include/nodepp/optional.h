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

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { 
template< class T > class optional_t {
protected: ptr_t<T> val;

public:

    optional_t( const T& value ) noexcept { val=ptr_t<T>( 0UL, value ); }

    optional_t()  /*----------*/ noexcept {}

    optional_t( null_t ) /*---*/ noexcept {}

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return !val.null(); }

    bool has_value() /*-------*/ const noexcept { return !val.null(); }

    /*─······································································─*/

    T& value() const { 
    if( val.null() ){ NODEPP_THROW_ERROR("expected does not have a value"); }
    return *val; }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/