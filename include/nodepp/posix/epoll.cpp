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
#include <sys/epoll.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using EPOLLFD= struct epoll_event; }
namespace nodepp { class poll_t : public generator_t {
private:

    struct waiter { bool blk; bool out; }; 

    using ITEM    = ptr_t<int>;
    using CALLBACK= function_t<int>;
    
protected:

    struct NODE {
        queue_t<CALLBACK> queue;
        int        len,pd;
        ptr_t<EPOLLFD> ev;
    };  ptr_t<NODE>   obj;

    int event_loop_next_tick() const noexcept {
    auto x =obj->queue.get(); 
    if ( x==nullptr ){ return -1; }
        
        switch( x->data() ){
            case -1: obj->queue.erase(x); break;
            case  1: obj->queue.next();   break;
            default: /*----------------*/ break;
        } 
        
    return x->next==nullptr ? -1 : 1; }

    int append( const int& fd, const int flags, void* ptr ) const noexcept {
        EPOLLFD event; if( !limit::fileno_ready() ){ return -1; }
        event.data.fd=fd; event.data.ptr=ptr; event.events=flags;
        return epoll_ctl( obj->pd, EPOLL_CTL_ADD, fd, &event );
    }

    int remove( const int& fd ) const noexcept {
        EPOLLFD event; event.events=0; event.data.fd=fd; 
        return epoll_ctl( obj->pd, EPOLL_CTL_DEL, fd, &event );
    }

    int invoke( EPOLLFD x, int fd, int state ) const noexcept {
        if( x.data.ptr == nullptr ){ return -1; } return 1;
    }

public:

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } close( obj->pd ); }

    poll_t() : obj( new NODE() ) {
        obj->pd = epoll_create1(0); if( obj->pd==-1 )
      { throw except_t("Can't open more epoll fd"); }
        obj->ev.resize( limit::get_soft_fileno() );
    }

    /*─······································································─*/

    template< class T, class V >
    int add( T& inp, uchar imode, V& out, uchar omode, CALLBACK cb ) noexcept {
    //  return append( fd, EPOLLOUT | EPOLLET , cb );
    return 1; }

    template< class T >
    int add( T& inp, uchar imode, CALLBACK cb ) noexcept {
    return 1; }
    
    /*
    bool push_read  ( const int& fd, void* cb ) noexcept { return append( fd, EPOLLIN  | EPOLLET , cb ); }
    bool push_duplex( const int& fd, void* cb ) noexcept { 
         bool success_write= push_write( fd, cb );
         bool success_read = push_read ( fd, cb );
         return success_read && success_write;
    }
    */

    /*─······································································─*/

    int next() noexcept {
    coBegin

        if((obj->len=epoll_wait( obj->pd, &obj->ev, obj->ev.size(), 0 ))<=0 ){ coStay(1); }

        do{ auto y=0; while( y < obj->len ){ auto x = obj->ev[y];
            if( x.events & ( EPOLLERR | EPOLLHUP ) )
              { invoke( x, x.data.fd, NODEPP_POLL_ERROR ); }
            if( x.events & EPOLLOUT )
              { invoke( x, x.data.fd, NODEPP_POLL_WRITE ); }
            if( x.events & EPOLLIN )
              { invoke( x, x.data.fd, NODEPP_POLL_READ  ); }
        ++y; }} while(0); coStay(1);

        coYield(1); return event_loop_next_tick();

    coFinish }

};}

/*────────────────────────────────────────────────────────────────────────────*/