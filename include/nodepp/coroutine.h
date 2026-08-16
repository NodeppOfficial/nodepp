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

namespace nodepp { template< class... A > class coroutine_arg_t { 
private:

    using T = function_t<int,uchar_64&,uchar_32&,A...>;

protected:

    struct NODE {
        T        callback; 
        uchar_32 time =0 ;
        uchar_64 state=0 ;
    };  ptr_t<NODE> obj;

public:

    template< class V >
    coroutine_arg_t( const V& callback ) noexcept : obj( new NODE() ) { obj->callback = callback; }
    coroutine_arg_t() /*--------------*/ noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void set_state( uchar_64 value ) const noexcept { obj->state = value; }

    uchar_64 get_state() const noexcept { return obj->state; }

    /*─······································································─*/

    int operator()( const A&... args ) const {
        return obj->callback( obj->state, obj->time, args... );
    }
    
    int emit( const A&... args ) const {
        return obj->callback( obj->state, obj->time, args... );
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp    { using coroutine_t = coroutine_arg_t</*----*/>; 
struct co_state_t   { uchar    flag  =0; uchar_32 delay  =0; int state=0; };
struct generator_t  { uchar_32 _time_=0; uchar_64 _state_=0; };
namespace coroutine { enum STATE : uchar {
     CO_STATE_START = 0b00000001,
     CO_STATE_YIELD = 0b00000010,
     CO_STATE_BLOCK = 0b00000000,
     CO_STATE_DELAY = 0b00000100,
     CO_STATE_END   = 0b00001000
}; }}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace coroutine {

    inline coroutine_t add( function_t<int, uchar_64&, uchar_32&> callback ) {
        return coroutine_t( callback );
    }

    template< class... T, class V >
    inline coroutine_arg_t<T...> add( V callback ) {
        return coroutine_arg_t<T...>( callback );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace coroutine {
    inline co_state_t getno( int state=0, uchar_64 _state_=0, uchar_32 time=0 ){
    thread_local static co_state_t tmp; co_state_t out; 
    
    out = tmp; tmp = co_state_t {};

    tmp.state=_state_; if( time>0 ){ 
        tmp.delay=time; /*------------*/
        tmp.flag =STATE::CO_STATE_DELAY; goto DONE; 
    } switch(state) {
        case -1: tmp.flag=STATE::CO_STATE_END  ; break;
        case  0: tmp.flag=STATE::CO_STATE_BLOCK; break;
        case  1: tmp.flag=STATE::CO_STATE_YIELD; break;
        default: tmp.flag=STATE::CO_STATE_START; break;
    }
    
    DONE:; return out; }
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/