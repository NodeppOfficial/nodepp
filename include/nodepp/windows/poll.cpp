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

#include <winsock.h>
#include <ws2tcpip.h>
#include <winsock2.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using POLLFD = WSAPOLLFD; class poll_t: public generator_t {
protected:

    struct NODE {
        array_t<POLLFD> ev;
        ptr_t<int>      ls;
    };  ptr_t<NODE>    obj;

    void remove( int fd ) const noexcept { obj->ev.erase( fd ); }

public:

    wait_t<ptr_t<int>> onEvent;
    wait_t<int>        onWrite;
    wait_t<int>        onError;
    wait_t<int>        onRead;

public: poll_t() noexcept : obj( new NODE() ) {}

   ~poll_t() noexcept { 
        if ( obj.count() > 1 ){ return; }
        for( auto x : obj->ev ) onError.emit(x.fd); 
    }

    /*─······································································─*/

    ptr_t<int> get_last_poll() const noexcept { return obj->ls; }

    /*─······································································─*/

    int next () noexcept { return emit(); }

    int emit () noexcept { 
        static ulong s=0; static POLLFD x;
    gnStart

        if( obj->ev.empty() )                                  { coEnd; }
        if( ::WSAPoll( obj->ev.data(), obj->ev.size(), 0 )<=0 ){ coEnd; } 
        
        s = obj->ev.size(); while( s-->0 ){ x = obj->ev[s]; 
              if( x.revents & POLLERR ){ remove(s); onError.emit(x.fd); obj->ls={{-1, (int)x.fd }}; onEvent.emit(obj->ls); coNext; }
            elif( x.revents & POLLIN  ){ remove(s);  onRead.emit(x.fd); obj->ls={{ 0, (int)x.fd }}; onEvent.emit(obj->ls); coNext; }
            elif( x.revents & POLLOUT ){ remove(s); onWrite.emit(x.fd); obj->ls={{ 1, (int)x.fd }}; onEvent.emit(obj->ls); coNext; }
        }
    
    gnStop
    };
    /*─······································································─*/

    bool push_write( const SOCKET& fd ) noexcept { 
         for( auto &x: obj->ev ){ if( x.fd==fd ){ return false; } }
	     obj->ev.unshift({ fd, POLLOUT, 0 }); return true;
    }

    bool push_read( const SOCKET& fd ) noexcept { 
         for( auto &x: obj->ev ){ if( x.fd==fd ){ return false; } }
         obj->ev.unshift({ fd, POLLIN, 0 }); return true;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/