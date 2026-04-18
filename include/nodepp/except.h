/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EXCEPT
#define NODEPP_EXCEPT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class except_t {
protected:

    string_t msg;

public:

    except_t( /*--*/ ) noexcept {}

    except_t( null_t ) noexcept {}

    /*─······································································─*/

    template< class T, class = typename type::enable_if<type::is_class<T>::value,T>::type >
    except_t( const T& except_type ) noexcept { msg = except_type.what(); }

    /*─······································································─*/

    except_t( const string_t& message ) noexcept { msg = message; }

    /*─······································································─*/

    template< class... T >
    except_t( const T&... message ) noexcept {
        msg = string::join( " ", message... );
    }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return !empty(); } 

    /*─······································································─*/

    void       print() const noexcept { console::error(msg); }
    bool       empty() const noexcept { return msg.empty(); }
    const char* what() const noexcept { return msg.c_str(); }
    operator   char*() const noexcept { return (char*)what(); }
    string_t    data() const noexcept { return msg; }
    string_t   value() const noexcept { return msg; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
