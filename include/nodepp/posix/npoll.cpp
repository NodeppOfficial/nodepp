/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class poll_t: public generator_t {
protected:

    struct POLLFD { int fd; int md; }; 

    struct NODE {
        queue_t<POLLFD> ev;
        ptr_t<int>      ls;
    };  ptr_t<NODE>    obj;

public:

    wait_t<ptr_t<int>> onEvent;
    wait_t<int>        onWrite;
    wait_t<int>        onError;
    wait_t<int>        onRead;

public: poll_t() noexcept : obj( new NODE() ) {}

   ~poll_t() noexcept { 
        if ( obj.count() > 1 ){ return; }
        for( auto x : obj->ev.data() ) 
           { onError.emit( x.md ); }
    }

    /*─······································································─*/

    ptr_t<int> get_last_poll() const noexcept { 
    ptr_t<int> result=obj->ls; return result; }

    /*─······································································─*/

    int next () noexcept { 
        static POLLFD x;
    coBegin
    
        if( obj->ev.empty() ){ coEnd; } while ( obj->ev.next() ) { x=obj->ev.get()->data;
          if( x.md == 1 ){ onWrite.emit(x.fd); obj->ls={{ 1, x.fd }}; onEvent.emit(x.fd); obj->ev.erase(obj->ev.get()); coNext; }
        elif( x.md == 0 ){  onRead.emit(x.fd); obj->ls={{ 0, x.fd }}; onEvent.emit(x.fd); obj->ev.erase(obj->ev.get()); coNext; }
        else             { onError.emit(x.fd); obj->ls={{-1, x.fd }}; onEvent.emit(x.fd); obj->ev.erase(obj->ev.get()); coNext; }
        }

    coFinish
    };

    /*─······································································─*/

    bool push_write( const int& fd ) noexcept {
        auto n=obj->ev.first();          while( n != nullptr  ){ 
        if( n->data.fd==fd )                  { return false; } 
        n = n->next; } obj->ev.push({ fd, 1 }); return true ;
    }

    bool push_read( const int& fd ) noexcept {
        auto n=obj->ev.first();          while( n != nullptr  ){ 
        if( n->data.fd==fd )                  { return false; } 
        n = n->next; } obj->ev.push({ fd, 0 }); return true ;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/