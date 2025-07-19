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

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } /*free();*/ }

    /*─······································································─*/

    int next () noexcept { 
    POLLFD x ; coBegin ; if( obj->ev.empty() ){ coEnd; } x = obj->ev.last()->data;
            
          if( x.md & 1 ){ onWrite.emit(x.fd); obj->ls={{ 1,x.fd}}; onEvent.emit(obj->ls); }
        elif( x.md & 4 ){  onRead.emit(x.fd); obj->ls={{ 0,x.fd}}; onEvent.emit(obj->ls); }
        else            { onError.emit(x.fd); obj->ls={{-1,x.fd}}; onEvent.emit(obj->ls); }

    obj->ev.pop() ; coGoto(0) ; coFinish
    };

    /*─······································································─*/

    bool push_write( const int& fd ) noexcept { obj->ev.unshift({fd,1}); return true; }

    bool push_read ( const int& fd ) noexcept { obj->ev.unshift({fd,4}); return true; }

    ptr_t<int> get_last_poll() const noexcept { return obj->ls; }

};}

/*────────────────────────────────────────────────────────────────────────────*/