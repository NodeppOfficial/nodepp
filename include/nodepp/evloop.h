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

namespace nodepp { namespace process {

    kernel_t& NODEPP_EVLOOP(){ thread_local static kernel_t out; return out; }
    invoke_t& NODEPP_INVOKE(){ thread_local static invoke_t out; return out; }
    
    /*─······································································─*/

    template< class... T >
    void revoke( const T&... args ){ NODEPP_INVOKE().off( args... ); }

    template< class... T >
    int call( const T&... args ){ return NODEPP_INVOKE().emit( args... ); }

    template< class... T >
    string_t invoke( const T&... args ){ return NODEPP_INVOKE().add( args... ); }
    
    /*─······································································─*/

    template< class... T >
    void await( const T&... args ){ while(NODEPP_EVLOOP().await( args... )==1){/*unused*/} }

    template< class... T >
    ptr_t<task_t> loop( const T&... args ){ return NODEPP_EVLOOP().loop_add( args... ); }

    template< class... T >
    ptr_t<task_t> poll( const T&... args ){ return NODEPP_EVLOOP().poll_add( args... ); }

    template< class... T >
    ptr_t<task_t> add ( const T&... args ){ return NODEPP_EVLOOP().loop_add( args... ); }
    
    /*─······································································─*/

    inline void clear( ptr_t<task_t> address ){ NODEPP_EVLOOP().off( address ); }
    inline void   off( ptr_t<task_t> address ){ NODEPP_EVLOOP().off( address ); }
    inline int   wake() /*-----------------*/ { return NODEPP_EVLOOP().wake (); }

    /*─······································································─*/

    inline bool should_close(){ return NODEPP_EVLOOP().empty() || *NODEPP_EVLOOP().should_close(); }
    inline bool        empty(){ return NODEPP_EVLOOP().empty(); }
    inline ulong        size(){ return NODEPP_EVLOOP().size (); }
    inline void        clear(){ /*--*/ NODEPP_EVLOOP().clear(); }

    /*─······································································─*/

    inline int next(){ return NODEPP_EVLOOP().next(); }

    inline void exit( int err=0 ){ 
        if( should_close() ) /*--------*/ { goto DONE; }
        *NODEPP_EVLOOP().should_close() = true; clear(); 
    DONE:; ::exit(err);  }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/