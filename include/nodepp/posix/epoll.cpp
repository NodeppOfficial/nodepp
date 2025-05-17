/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <sys/epoll.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using EPOLLFD = struct epoll_event; class poll_t: public generator_t {
protected:

    struct NODE {
        ptr_t<int>     ls;
        ptr_t<EPOLLFD> ev;
        int            pd;
    };  ptr_t<NODE>   obj;

    void remove( EPOLLFD x ) const noexcept {
         epoll_ctl( obj->pd, EPOLL_CTL_DEL, x.data.fd, &x ); 
    }

public:

    wait_t<ptr_t<int>> onEvent;
    wait_t<int>        onWrite;
    wait_t<int>        onError;
    wait_t<int>        onRead;

public: 

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } close( obj->pd ); }

    poll_t() : obj( new NODE() ) {
        obj->pd = epoll_create1(0); if( obj->pd == -1 )
        process::error("Can't open an epoll fd");
        obj->ev.resize( MAX_FILENO );
    }

    /*─······································································─*/

    ptr_t<int> get_last_poll() const noexcept { return obj->ls; }

    /*─······································································─*/

    int next () noexcept { return emit(); }

    int emit () noexcept { 
        static int c=0; static EPOLLFD x;
    coStart

        if( (c=epoll_wait( obj->pd, &obj->ev, obj->ev.size(), 0 ))<=0 ) { coEnd; } while( c-->0 ){ x = obj->ev[c]; 
              if( x.events & EPOLLIN  ){ remove(x);  onRead.emit(x.data.fd); obj->ls={{ 0, x.data.fd }}; onEvent.emit(obj->ls); coNext; }
            elif( x.events & EPOLLOUT ){ remove(x); onWrite.emit(x.data.fd); obj->ls={{ 1, x.data.fd }}; onEvent.emit(obj->ls); coNext; }
            else                       { remove(x); onError.emit(x.data.fd); obj->ls={{-1, x.data.fd }}; onEvent.emit(obj->ls); coNext; }
        }

    coStop
    };

    /*─······································································─*/

    bool push_write( const int& fd ) noexcept { 
         EPOLLFD event; 
                 event.data.fd = fd;
                 event.events  = EPOLLOUT;
         return epoll_ctl( obj->pd, EPOLL_CTL_ADD, fd, &event )!=-1;
    }

    bool push_read( const int& fd ) noexcept { 
         EPOLLFD event; 
                 event.data.fd = fd;
                 event.events  = EPOLLIN;
         return epoll_ctl( obj->pd, EPOLL_CTL_ADD, fd, &event )!=-1;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/