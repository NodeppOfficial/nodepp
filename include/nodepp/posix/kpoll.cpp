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

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using KPOLLFD = struct kevent; class poll_t: public generator_t {
protected:

    struct NODE {
        ptr_t<int>     ls;
        ptr_t<KPOLLFD> ev;
        int            pd;
    };  ptr_t<NODE>   obj;

    void remove( KPOLLFD x ) const noexcept {
         EV_SET( &x, x.ident, 0, EV_DELETE, 0, 0, NULL ); 
         kevent( obj->pd, &x, 1, NULL, 0, NULL );
    }

public:

    wait_t<ptr_t<int>> onEvent;
    wait_t<int>        onWrite;
    wait_t<int>        onError;
    wait_t<int>        onRead;

public: 

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } close( obj->pd ); }

    poll_t() : obj( new NODE() ) {
        obj->pd = kqueue(0); if( obj->pd == -1 )
        process::error("Can't open an epoll fd");
        obj->ev.resize( MAX_FILENO );
    }

    /*─······································································─*/

    ptr_t<int> get_last_poll() const noexcept { return obj->ls; }

    /*─······································································─*/

    int next () noexcept { return emit(); }

    int emit () noexcept { 
        static int c=0; static KPOLLFD x;
    gnStart

        if( (c=kevent( obj->pd, NULL, 0, &obj->ev, obj->ev.size(), 0 ))<=0 ) { coEnd; } while( c-->0 ){ x = obj->ev[c]; 
              if( x.flags & EVFILT_READ  ){ remove(x);  onRead.emit(x.ident); obj->ls={{ 0, x.ident }}; onEvent.emit(obj->ls); coNext; }
            elif( x.flags & EVFILT_WRITE ){ remove(x); onWrite.emit(x.ident); obj->ls={{ 1, x.ident }}; onEvent.emit(obj->ls); coNext; }
            else                          { remove(x); onError.emit(x.ident); obj->ls={{-1, x.ident }}; onEvent.emit(obj->ls); coNext; }
        }

    gnStop
    };

    /*─······································································─*/

    bool push_write( const int& fd ) noexcept { KPOLLFD event;
	     EV_SET( &event, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL );
         return kevent( obj->pd, &event, 1, NULL, 0, NULL )!=-1;
    }

    bool push_read( const int& fd ) noexcept { KPOLLFD event;
	     EV_SET( &event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL );
         return kevent( obj->pd, &event, 1, NULL, 0, NULL )!=-1;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/