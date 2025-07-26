/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_DEBUG
#define NODEPP_DEBUG

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class debug_t {     
protected: 

    struct NODE { 
        string_t msg;
        void* ev = nullptr;
    };  ptr_t<NODE> obj;

public: debug_t() noexcept : obj(new NODE()) { }
    
    /*─······································································─*/

    virtual ~debug_t() noexcept { 
        if ( obj.count() == 2 ){ 
	         console::log( obj->msg, "closed" );  
        }    process::onSIGERR.off( obj->ev );
    }
    
    /*─······································································─*/
    
    debug_t( const string_t& msg ) noexcept : obj(new NODE()) {
        obj->msg = msg; auto inp = type::bind( this );
        obj->ev  = process::onSIGERR([=](){ inp->error(); });
	               console::log( obj->msg, "open" );
    }
    
    /*─······································································─*/

    template< class... T >
    void log( const T&... args ) const noexcept { console::log( "--", args... ); }

    void error() const noexcept { console::error( obj->msg ); }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif