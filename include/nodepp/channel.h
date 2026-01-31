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

#include "any.h"
#include "mutex.h"
#include "worker.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class channel_t: public generator_t {
private:

    struct NODE { 
        /*--------*/ queue_t<T> queue; 
        ulong limit; mutex_t mut; 
    }; ptr_t<NODE> obj;

public:

    channel_t( ulong limit=0 ) noexcept : obj( new NODE() ){ obj->limit=limit; }

    /*─······································································─*/

    bool is_empty() const noexcept { 
    bool empty = true; obj->mut.emit([&](){ 
         empty = obj->queue.empty(); 
    return -1; }); return empty; }

    ulong size() const noexcept { 
    ulong count = 0; obj->mut.emit([&](){ 
          count = obj->queue.size(); 
    return -1; }); return count; }

    /*─······································································─*/

    int _read( ptr_t<T>& out ) const noexcept { 
    return obj->mut._emit([&](){ 
        if( obj->queue.empty() ){ return -2; }
        out=obj->queue.first()->data; obj->queue.shift();
    return 1; }); }

    int _write( const T& msg ) const noexcept { 
    return obj->mut._emit([&](){ 
        if( obj->limit>0 && obj->queue.size()>=obj->limit )
          { return -2; } obj->queue.push( msg ); 
    return 1; }); }

    /*─······································································─*/

    ptr_t<T> read() const noexcept { 
    ptr_t<T> out; int c=0; while((c=obj->mut.emit([&](){
        if( obj->queue.empty() ){ return -2; }
        out=obj->queue.first()->data; obj->queue.shift();
    return 1; }))==-2 ){ process::next(); } return c>0 ? out : nullptr ; }

    int write( const T& msg ) const noexcept { 
    int c=0; while((c=obj->mut.emit([&](){ 
        if( obj->limit>0 && obj->queue.size()>=obj->limit )
          { return -2; } obj->queue.push( msg ); 
    return 1; }))==-2){ process::next(); } return c>0 ? 1 : -1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif