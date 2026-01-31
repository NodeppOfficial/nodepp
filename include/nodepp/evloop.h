/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT_LOOP
#define NODEPP_EVENT_LOOP

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { atomic_t<bool> _EXIT_ = false;

    kernel_t& NODEPP_EV_LOOP(){ thread_local static kernel_t evloop; return evloop; }
    
    /*─······································································─*/

    template< class... T >
    void await( const T&... args ){ while(NODEPP_EV_LOOP().await( args... )==1){/*unused*/} }

    template< class... T >
    ptr_t<task_t> foop( const T&... args ){ return NODEPP_EV_LOOP().loop_add( args... ); }

    template< class... T >
    ptr_t<task_t> loop( const T&... args ){ return NODEPP_EV_LOOP().loop_add( args... ); }

    template< class... T >
    ptr_t<task_t> poll( const T&... args ){ return NODEPP_EV_LOOP().poll_add( args... ); }

    template< class... T >
    ptr_t<task_t> add ( const T&... args ){ return NODEPP_EV_LOOP().loop_add( args... ); }
    
    /*─······································································─*/

    inline void clear( ptr_t<task_t> address ){ NODEPP_EV_LOOP().off( address ); }
    inline void   off( ptr_t<task_t> address ){ NODEPP_EV_LOOP().off( address ); }
    inline int   emit() /*-----------------*/ { return NODEPP_EV_LOOP().emit (); }

    /*─······································································─*/

    inline bool should_close(){ return NODEPP_EV_LOOP().empty() || _EXIT_.get(); }
    inline bool        empty(){ return NODEPP_EV_LOOP().empty(); }
    inline ulong        size(){ return NODEPP_EV_LOOP().size (); }
    inline void        clear(){ /*--*/ NODEPP_EV_LOOP().clear(); }

    /*─······································································─*/

    inline int next(){ return NODEPP_EV_LOOP().next(); }

    inline void exit( int err=0 ){ 
        if( should_close() ){ goto DONE; }
        _EXIT_.set(true); clear(); DONE:; ::exit(err); 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/