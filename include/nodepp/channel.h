/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_CHANNEL
#define NODEPP_CHANNEL

/*────────────────────────────────────────────────────────────────────────────*/

#include "mutex.h"
#include "worker.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class channel_t {
private:

    struct NODE { 
        /*--------*/ queue_t<T> queue; 
        ulong limit; mutex_t mut; 
    };  atomic_ptr_t<NODE> obj;

public:

    channel_t( ulong limit=0 ) noexcept : obj( new NODE() ) { obj->limit=limit; }

    /*─······································································─*/

    bool empty() const noexcept { return obj->queue.empty(); }

    ulong size() const noexcept { return obj->queue.size (); }

    /*─······································································─*/

    void free () const noexcept { clear(); }
    void clear() const noexcept { 
    obj->mut.lock([&](){ obj->queue.clear(); }); }

    /*─······································································─*/

    ptr_t<T> read() const noexcept { 
    ptr_t<T> out; obj->mut.lock([&](){ do {
        if( obj->queue.empty() ){ break; } 
        out=obj->queue.data (); obj->queue.clear(); 
    } while(0); }); return out; }

    template< class... V >
    int write( const V&... args ) const noexcept { 
    int x=0; obj->mut.lock([&](){ iterator::some ([&]( T clb ){ do {
        
        if( obj->limit>0 && obj->queue.size()>=obj->limit )
          { break; } obj->queue.push( clb ); x++;
    
    return false; } while(0); return true; }, args... ); }); return x; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif