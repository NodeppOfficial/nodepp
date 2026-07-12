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

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {
template <typename T, typename E> struct expected_t { 
protected: 

    pair_t<ptr_t<T>,ptr_t<E>> val; 
    
public:

    expected_t( const T& value ) noexcept { val.first =ptr_t<T>( 0UL, value ); }

    expected_t( const E& error ) noexcept { val.second=ptr_t<E>( 0UL, error ); }

    expected_t( null_t ) /*---*/ noexcept {}

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return !val.first.null(); }

    bool has_value() /*-------*/ const noexcept { return !val.first.null(); }

    /*─······································································─*/

    T& value() const { if( val.first.null() ){ 
        NODEPP_THROW_ERROR("expected does not have a value"); 
    }   return *val.first; }

    /*─······································································─*/

    E& error() const { if( val.second.null() ){ 
        NODEPP_THROW_ERROR("expected does not have a value"); 
    }   return *val.second; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/