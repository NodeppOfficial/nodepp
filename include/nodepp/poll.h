/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POLL
#define NODEPP_POLL

#if   _KERNEL == NODEPP_KERNEL_WINDOWS
        #include "wait.h"
        #include "windows/poll.cpp"
#elif _KERNEL == NODEPP_KERNEL_POSIX
    #if   _POLL == NODEPP_POLL_EPOLL
        #include "wait.h"
        #include "posix/epoll.cpp"
    #elif _POLL == NODEPP_POLL_KPOLL
        #include "wait.h"
        #include "posix/kpoll.cpp"
    #else
        #include "wait.h"
        #include "posix/poll.cpp"
    #endif
#else
    #include "wait.h"
    #define NODEPP_NO_POLL
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_NO_POLL

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

    virtual ~poll_t() noexcept { 
        if ( obj.count() > 1 ){ return; }
        for( auto x : obj->ev.data() ) 
             onError.emit( x.md ); 
    }

    /*─······································································─*/

    ptr_t<int> get_last_poll() const noexcept { 
        ptr_t<int> result = obj->ls; return result; 
    }

    /*─······································································─*/

    int next () noexcept { return emit(); }

    int emit () noexcept { 
        static POLLFD x;
    gnStart
    
        if( obj->ev.empty() ){ coEnd; } while ( obj->ev.next() ) { x=obj->ev.get()->data;
            if( x.md == 1 ){ obj->ev.erase(obj->ev.get()); onWrite.emit(x.fd); obj->ls={{ 1, x.fd }}; onEvent.emit(obj->ls); coNext; }
          elif( x.md == 0 ){ obj->ev.erase(obj->ev.get());  onRead.emit(x.fd); obj->ls={{ 0, x.fd }}; onEvent.emit(obj->ls); coNext; }
          else             { obj->ev.erase(obj->ev.get()); onError.emit(x.fd); obj->ls={{-1, x.fd }}; onEvent.emit(obj->ls); coNext; }
        }

    gnStop
    };

    /*─······································································─*/

    bool push_write( const int& fd ) noexcept { 
         auto n=obj->ev.first(); while( n==nullptr ){ 
          if( n->data.fd==fd ) { return false; } 
              n = n->next;
         }  obj->ev.push({ fd, 1 }); return true;
    }

    bool push_read( const int& fd ) noexcept { 
         auto n=obj->ev.first(); while( n==nullptr ){ 
          if( n->data.fd==fd ) { return false; } 
              n = n->next;
         }  obj->ev.push({ fd, 0 }); return true;
    }

};}

#undef NODEPP_NO_POLL
#endif