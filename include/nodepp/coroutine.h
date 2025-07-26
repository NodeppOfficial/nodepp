/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_COROUTINE
#define NODEPP_COROUTINE

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { struct generator_t { protected: 
    ulong _time_ = 0; int _state_= 0; 
}; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class coroutine_t { 
private:

    using T = function_t<int,int&,ulong&>;

protected:

    struct NODE {
        T   callback; 
        ulong time=0;
        int state =0;
        bool alive=1;
    };  ptr_t<NODE> obj;

public:

    coroutine_t( T callback ) : obj( new NODE() ) { obj->callback = callback; }

    coroutine_t() : obj( new NODE() ) { obj->alive = 0; }

    virtual ~coroutine_t() noexcept {}
    
    /*─······································································─*/

    void off() const noexcept { obj->alive = 0; obj->callback.free(); }

    void set_state( int value ) const noexcept { obj->state = value; }

    int get_state() const noexcept { return obj->state; }

    void free() const noexcept { off(); }

    /*─······································································─*/

    bool is_closed() const noexcept { return !is_available(); }

    bool is_available() const noexcept { return obj->alive; }
    
    /*─······································································─*/

    coEmit() const noexcept { return next(); }

    int next() const noexcept {
        if( !obj->alive ){ return -1; }
        return obj->callback( obj->state, obj->time );
    }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace coroutine {
    coroutine_t add( function_t<int,int&,ulong&> callback ) {
        return coroutine_t( callback );
    }
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif