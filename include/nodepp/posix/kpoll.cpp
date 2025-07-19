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
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using KPOLLFD = struct kevent; class poll_t {
protected:

    struct NODE {
        int        len,pd;
        ptr_t<KPOLLFD> ev;
    };  ptr_t<NODE>   obj;

    using T = function_t< void, void*, int, int >;

    int invoke( KPOLLFD x, int fd, int state ) const noexcept {
        if( x.data.ptr == nullptr ){ return -1; }
        type::cast<T>(x.data.ptr)->( x.data.ptr, fd, state );
    }

public:

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } close( obj->pd ); }

    poll_t() : obj( new NODE() ) {
        obj->pd = kqueue(0); if( obj->pd==-1 )
      { throw except_t("Can't open more kqueue fd"); }
        obj->ev.resize( limit::get_soft_fileno() );
    }

    /*─······································································─*/

    bool push_write ( const int& fd, T* cb ) noexcept { return append( fd, EVFILT_WRITE | EV_CLEAR, cb ); }
    bool push_read  ( const int& fd, T* cb ) noexcept { return append( fd, EVFILT_READ  | EV_CLEAR, cb ); }
    bool push_duplex( const int& fd, T* cb ) noexcept { 
         bool success_write= push_write( fd, cb );
         bool success_read = push_read ( fd, cb );
         return success_read && success_write;
    }

    /*─······································································─*/

    int next() noexcept {

        if((obj->len=kevent( obj->pd, NULL, 0, &obj->ev, obj->ev.size(), 0 ))<=0 ){ return -2; }

        auto y=0; while( y < obj->len ){ auto x = obj->ev[y];
            if( x.flags & (EV_ERROR|EV_EOF) )
              { invoke( x, x.data.fd, 0x00 ); }
            if( x.filter& EVFILT_WRITE )
              { invoke( x, x.data.fd, 0x02 ); }
            if( x.filter& EVFILT_READ  )
              { invoke( x, x.data.fd, 0x01 ); }
        ++y; }

    return obj->len; }

    /*─······································································─*/

    int append( const int& fd, const int flags, T* cb ) const noexcept {
        KPOLLFD event; event.udata = cb;
        EV_SET( &event, fd, flags, EV_ADD|EV_ENABLE, 0, 0, NULL );
        return kevent( obj->pd, &event, 1, NULL, 0, NULL );
    }

    int remove( const int& fd ) const noexcept { 
        KPOLLFD event; event.udata = cb;
        EV_SET( &event, fd, 0, EV_DELETE|EV_DISABLE, 0, 0, NULL );
        return kevent( obj->pd, &event, 1, NULL, 0, NULL );
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/