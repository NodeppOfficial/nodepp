/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_IOCP
#define NODEPP_POSIX_IOCP

#if _KERNEL_ == NODEPP_KERNEL_WINDOWS

#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #if   ( _OS_ == NODEPP_OS_FRBSD ) || ( _OS_ == NODEPP_OS_APPLE )
    #elif ( _OS_ == NODEPP_OS_LINUX )
    #else
    #endif
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class kernel_t {
private:

    class fd_t { public:

        struct NODE {
            any_t user_data; int fd; function_t<void> cb;
        };  ptr_t<NODE> obj;

        fd_t( int fd, function_t<void,fd_t*> cb, any_t ud ) : obj ( new NODE() )
            { user_data = ud; obj->cb = cb; }

        virtual ~fd_t(  ) noexcept { if( obj.count()>1 ){ return } obj->cb(this); }

    };

    class evfd_t: public fd_t { public:
        
        evfd_t( int fd, function_t<void,fd_t*> cb, any_t ud ) : fd_t( fd, cb, ud ) {}

        void emit() const noexcept {  }

    };

protected:

    struct NODE { queue_t<fd_t> queue; }; ptr_t<NODE> obj;

public:

    int append( const int fd, const int flags, void* ptr ) const noexcept {
        EPOLLFD event; /*--------------------------------------*/
        event.data.fd=fd; event.data.ptr=ptr; event.events=flags;
        return epoll_ctl( obj->pd, EPOLL_CTL_ADD, fd, &event );
    }

    fd_t set_event() const noexcept {

        auto self = type::bind( this ); auto fd_t fd(
            eventfd( 0, EFD_CLOEXEC | EFD_NONBLOCK ),
            [=]( fd_t* fd ){ 
                self->remove(  );
                ::close( fd->obj->fd ); 
            }, nullptr
        );

        int efd = ;

        ev.events = EPOLLIN; // Default Level-Triggered
        ev.data.fd = fd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    }


};}