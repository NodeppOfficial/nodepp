/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EXPECTED
#define NODEPP_EXPECTED

namespace nodepp {
template <typename T, typename E> struct expected_t { 
protected:

    struct NODE {
        bool has;
        T data; E err;
    };  ptr_t<NODE> obj;

public:

    expected_t( const T& val ) : obj( new NODE() ) {
        obj->has = true; obj->data = val;
    }

    expected_t( const E& err ) : obj( new NODE() ) {
        obj->has = false; obj->err = err; 
    }

    /*─······································································─*/

    bool has_value() const noexcept { 
        if( obj == nullptr ){ return false; }
            return obj->has;
    }

    /*─······································································─*/

    T& value() const { if ( !has_value() ) {
        process::error("expected does not have a value");
    }   return obj->data;  }

    /*─······································································─*/

    E& error() const { if ( has_value() ) {
        process::error("expected does not have a value");
    }   return obj->err;  }

};}

#endif