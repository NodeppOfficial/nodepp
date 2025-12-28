/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_KERNEL
#define NODEPP_KERNEL
#if  ( _OS_ == NODEPP_OS_FRBSD ) || ( _OS_ == NODEPP_OS_APPLE )
    #define NODEPP_POLL_KQUEUE
#elif( _OS_ == NODEPP_OS_LINUX )
    #define NODEPP_POLL_EPOLL
#elif( _OS_ == NODEPP_OS_WINDOWS )
    #define NODEPP_POLL_IOCP
#else
    #define NODEPP_POLL_POLL
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if define( NODEPP_POLL_EPOLL )

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include "../loop.h"

namespace nodepp { class kernel_t: public generator_t {
private:

    enum STATE{ 
         WRITE  = EPOLLOUT /*------*/ | EPOLLET,
         READ   = EPOLLIN  /*------*/ | EPOLLET,
         DUPLEX = EPOLLIN  | EPOLLOUT | EPOLLET
    };

    enum FLAG {
         FD_FLAG_UNKNOWN = 0x00000000,
         FD_FLAG_STREAM  = 0x00000001,
         FD_FLAG_EVENT   = 0x00000010,
         FD_FLAG_TIMER   = 0x00000100,
         FD_FLAG_RAW     = 0x00001000,
    };

    using EPOLLFD = struct epoll_event;
    using EPOLLCB = function_t<int>;

    /*─······································································─*/

    class kevent_t { public:

        struct NODE { any_t user_data; int fd, flag; }; ptr_t<NODE> obj;

        any_t get() const noexcept { return obj->user_data; }

        kevent_t( int fd, int flag, any_t ud ) : obj ( new NODE() )
                { user_data = ud; obj->flag = flag; }

    };

    /*─······································································─*/

    void* append_timer_interval( any_t user_data, ulong time_ms ) const noexcept {
        int tfd = timerfd_create( CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK );
        if( tfd == -1 ){ return nullptr; }

        struct itimerspec ts;
        ts.it_interval.tv_sec  = time_ms / 1000 ; 
        ts.it_value   .tv_sec  = ts.it_interval.tv_sec ;
        ts.it_interval.tv_nsec = (time_ms % 1000) * 1e6;
        ts.it_value   .tv_nsec = ts.it_interval.tv_nsec;

        if( timerfd_settime( tfd, 0, &ts, NULL )==-1 ) 
          { ::close( tfd ); return nullptr; }

        kevent_t kv( tfd, FLAG::FD_FLAG_TIMER, user_data );
        /*-----*/ obj->kv_queue.push( kv );
        auto id = obj->kv_queue.last();
        
        append( fd.obj->fd, STATE::READ, &id );
    return id; }

    void* append_timer_timeout( any_t user_data, ulong time_ms ) const noexcept {
        int tfd = timerfd_create( CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK );
        if( tfd == -1 ){ return nullptr; }

        struct itimerspec ts;
        ts.it_interval.tv_sec  = 0;
        ts.it_interval.tv_nsec = 0; 
        ts.it_value.tv_sec     = time_ms / 1000;
        ts.it_value.tv_nsec    =(time_ms % 1000) * 1e6;

        if( timerfd_settime( tfd, 0, &ts, NULL )==-1 ) 
          { ::close( tfd ); return nullptr; }

        kevent_t kv( tfd, FLAG::FD_FLAG_TIMER, user_data );
        /*-----*/ obj->kv_queue.push( kv );
        auto id = obj->kv_queue.last();

        append( fd.obj->fd, STATE::READ, &id );
    return id; }

    void* append_writter( any_t user_data, int sfd ) const noexcept {
        if( sfd==-1 ){ return nullptr; }

        kevent_t kv( sfd, FLAG::FD_FLAG_STREAM, user_data );
        /*-----*/ obj->kv_queue.push( kv );
        auto id = obj->kv_queue.last();

        append( fd.obj->fd, STATE::WRITE, &id );
    return id; }

    void* append_reader( any_t user_data, int sfd ) const noexcept {
        if( sfd==-1 ){ return nullptr; }

        kevent_t kv( sfd, FLAG::FD_FLAG_STREAM, user_data );
        /*-----*/ obj->kv_queue.push( kv );
        auto id = obj->kv_queue.last();
        
        append( fd.obj->fd, STATE::READ, &id );
    return id; }

    void* append_event( any_t user_data ) const noexcept {
        int efd = eventfd( 0, EFD_CLOEXEC | EFD_NONBLOCK );
        if( efd == -1 ){ return nullptr; }

        kevent_t kv( tfd, FLAG::FD_FLAG_EVENT, user_data );
        /*-----*/ obj->kv_queue.push( kv );
        auto id = obj->kv_queue.last();

        append( fd.obj->fd, STATE::READ, &id );
    return id; }

    /*─······································································─*/

    int append( const int fd, const int flags, void* ptr ) const noexcept {
        EPOLLFD event;
        event.data.fd=fd; event.data.ptr=ptr; event.events=flags;
        return epoll_ctl( obj->pd, EPOLL_CTL_ADD, fd, &event );
    }

    int remove( void* ptr ) const noexcept {
        auto pt = obj->fd_queue.as( ptr );
        auto fd = pt->data.first; obj->queue.erase(pt);
        EPOLLFD event; event.data.fd=fd; event.events=0;
        return  epoll_ctl( obj->pd, EPOLL_CTL_DEL, fd, &event );
    }

protected:

    struct NODE { 
        queue_t<kevent_t> kv_queue;
        queue_t<EPOLLFD>  fd_queue;
        loop_t ev_queue,  fs_queue;
    };  ptr_t<NODE> obj;

public:

    /*─······································································─*/

    /*─······································································─*/

    /*─······································································─*/

    inline int next() noexcept {
    coBegin

        coWait( obj->ev_queue.next()>=0 ){ coNext; } process::set_timeout( obj->ev_queue.get_delay() );
        coWait( obj->fs_queue.next()>=0 ){ coNext; } process::set_timeout( obj->fs_queue.get_delay() );

        if((obj->len=epoll_wait( obj->pd, &obj->ev, obj->ev.size(), TIMEOUT ))<=0 )
          { coEnd; } obj->y=0;

        while( obj->y < obj->len ){ do {
            
            auto x = obj->ev[ obj->y ];
            auto y = obj->queue.as( x.data.ptr );

            if( x.events & ( EPOLLERR | EPOLLHUP ) &&
              ( x.events & ( EPOLLIN  | EPOLLOUT ))==0
            ) { remove( y ); ++obj->y; break; }

            switch( type::cast<NODE_CLB>( y->data.second ).emit() ){
                case -1: remove( y ); ++obj->y; break;
                case  0: /*------------------*/ break;
                default: /*--------*/ ++obj->y; break;
            }
        
        } while(0); coNext; }

    coFinish }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if define( NODEPP_POLL_KQUEUE )

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if define( NODEPP_POLL_IOCP )
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if define( NODEPP_POLL_POLL )

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/poll.h>

#endif

/*────────────────────────────────────────────────────────────────────────────*/