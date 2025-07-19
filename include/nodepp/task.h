/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TASK
#define NODEPP_TASK

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { loop_t loop; poll_t poll;

    /*─······································································─*/

    ulong size(){ return _TASK_ + loop.size(); }

    void clear(){ _TASK_=0; loop.clear(); }

    bool empty(){ return size() <= 0; }

    /*─······································································─*/

    bool should_close(){ return empty() || _EXIT_; }

    int next(){ coStart
        coWait( loop.next()==1 );
        coWait( poll.next()==1 );
        process::yield();
    coStop }

    /*─······································································─*/

    void clear( void* address ){
         if( address == nullptr ){ return; }
         memset( address, 0, sizeof(bool) );
    }

    /*─······································································─*/

    template< class... T >
    void* add( const T&... args ){ return loop.add( args... ); }

    template< class T, class... V >
    void await( T cb, const V&... args ){ while( ([&](){

        switch( cb( args... ) ){
            case  1: next(); return 1; break;
            case  0: /*---*/ return 0; break;
            default: /*-------------*/ break;
        }

    return -1; })()>=0 ){} }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
