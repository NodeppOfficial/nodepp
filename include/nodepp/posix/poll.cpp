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
#include <poll.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using POLLFD = struct pollfd; class poll_t: public generator_t {
protected:

    struct NODE {
        int        off,len;
        ptr_t<POLLFD>   ev;
    };  ptr_t<NODE>    obj;

public:

    poll_t() noexcept : obj( new NODE() ) { 
        obj->ev.resize( limit::get_soft_fileno() );
        obj->off=0; obj->len=0; 
    }

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } }

    /*─······································································─*/

    int next() noexcept {
        if((obj->len=poll( obj->ev.data(), obj->ev.size(), 0 ))<=0 ) 
          { return -2; } return obj->len;
    }

    /*─······································································─*/

    int check( const int& fd ) const noexcept { if( obj->len<=0 ){ return 0; }

        auto y=0; while( y < obj->len ){ auto x = obj->ev[y]; ++y;
        int  out= 0; if( x.data.fd == fd ){
                if( x.revents &(EPOLLERR|EPOLLHUP|EPOLLNVAL)){ return -1; }
                if( x.revents & EPOLLIN  ){ out |= 0x01; }
                if( x.revents & EPOLLOUT ){ out |= 0x02; } return out;
            }
        }

    return 0x04; }

    /*─······································································─*/

    int remove( const int& fd ) const noexcept { 
        if( obj->ev.empty() ){ return 0; } int y=0;
        auto x = obj->ev.size(); while( x-->0 ){
            if( obj->ev[x].data.fd == fd ){ obj->ev.erase(x); ++y; }
        }   return y;
    }

    /*─······································································─*/

    bool push_write( const int& fd ) noexcept { 
         if( obj->off >= obj->ev.size() ){ return false; }
         obj->ev[obj->off] = POLLFD({ fd, POLLOUT, 0 });
	     ++obj->off; return true;
    }

    bool push_read( const int& fd ) noexcept { 
         if( obj->off >= obj->ev.size() ){ return false; }
         obj->ev[obj->off] = POLLFD({ fd, POLLIN, 0 });
         ++obj->off; return true;
    }

    bool push_duplex( const int& fd ) noexcept { 
         bool success_write= push_write( fd );
         bool success_read = push_read ( fd );
         return success_read && success_write;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/