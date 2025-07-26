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

    struct NODE {
        void *ev = nullptr;
        string_t msg;
    };  ptr_t<NODE> obj;

public:

    virtual ~except_t() noexcept {
        if( obj->ev == nullptr ){ return; }
   	    process::onSIGERR.off( obj->ev );
    }

    except_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    template< class T, class = typename type::enable_if<type::is_class<T>::value,T>::type >
    except_t( const T& except_type ) noexcept : obj(new NODE()) {
        obj->msg = except_type.what(); auto inp = type::bind( this );
        obj->ev  = process::onSIGERR.once([=]( ... ){ inp->print(); });
    }

    /*─······································································─*/

    template< class... T >
    except_t( const T&... msg ) noexcept : obj(new NODE()) {
        obj->msg = string::join( " ", msg... ); auto inp = type::bind( this );
        obj->ev  = process::onSIGERR.once([=]( ... ){ inp->print(); });
    }

    /*─······································································─*/

    except_t( const string_t& msg ) noexcept : obj(new NODE()) {
        obj->msg = msg; auto inp = type::bind( this );
        obj->ev  = process::onSIGERR.once([=]( ... ){ inp->print(); });
    }

    /*─······································································─*/

    void       print() const noexcept { console::error(obj->msg); }
    bool       empty() const noexcept { return obj->msg.empty(); }
    const char* what() const noexcept { return obj->msg.c_str(); }
    operator   char*() const noexcept { return (char*)what(); }
    string_t    data() const noexcept { return obj->msg; }
    string_t   value() const noexcept { return obj->msg; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
