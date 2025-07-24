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

namespace nodepp { enum POLL_STATE {
    READ = 1, WRITE = 2, DUPLEX = 3
};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using EPOLLFD= struct epoll_event; }
namespace nodepp { class poll_t : public generator_t {
private:

    using CALLBACK = function_t<int>;
    using DATA     = type::pair<int,CALLBACK>;

    struct waiter { bool blk; bool out; }; 
    
protected:

    struct NODE {
        queue_t<DATA>  queue;
        map_t<int,void*> mem;
        int        len,pd;
        bool          blk;
        ptr_t<EPOLLFD> ev;
    };  ptr_t<NODE>   obj;

    /*─······································································─*/

    int emit() const noexcept { 
    auto z =obj->mem.raw(); if( z.empty()  ){ return -1; } 
    auto x =z.get(); /*--*/ if( x==nullptr ){ return -1; }
    auto y =obj->queue.as( x->data.second );
        
        obj->blk = 1; bool out = x->next==nullptr;
        
        switch( type::cast<CALLBACK>( y->data.second ).emit() ){
            case -1: remove( y ); z.next(); break;
            case  1: /*--------*/ z.next(); break;
            default: /*------------------*/ break;
        }

        obj->blk = 0;
    
    return out ? -1 : 1; }

    /*─······································································─*/
    
    bool listen( const int fd, const int flags, void* ptr ) noexcept { bool x=1, y=1;
        if( flags & POLL_STATE::READ  ){ x = append( fd, EPOLLIN  | EPOLLET, ptr )!=-1; }
        if( flags & POLL_STATE::WRITE ){ y = append( fd, EPOLLOUT | EPOLLET, ptr )!=-1; }
        return ( x && y );
    }

    int append( const int fd, const int flags, void* ptr ) const noexcept {
        EPOLLFD event; if( !limit::fileno_ready() ){ return -1; }
        event.data.fd=fd; event.data.ptr=ptr; event.events=flags;
        return epoll_ctl( obj->pd, EPOLL_CTL_ADD, fd, &event );
    }

    int remove( void* ptr ) const noexcept {
        auto pt = obj->queue.as( ptr );
        auto fd = pt->data.first; obj->queue.erase(pt);
        EPOLLFD event; event.data.fd=fd; event.events=0;
        return epoll_ctl( obj->pd, EPOLL_CTL_DEL, fd, &event );
    }

    /*─······································································─*/

    template< class T, class... V >
    void* push( int fd, T cb, const V&... arg ) const noexcept {

        ptr_t<waiter> tsk = new waiter();
        auto clb=type::bind( cb );
        tsk->blk=0; tsk->out=1; 

        obj->queue.push({ fd, [=](){
            if( tsk->out==0 ){ return -1; }
            if( tsk->blk==1 ){ return  1; } 
                tsk->blk =1; int rs=(*clb)( arg... );
            if( clb.null () ){ return -1; }  
                tsk->blk =0; return !tsk->out ? -1 : rs;
        } }); 
        
        return (void*) &tsk->out;
    }

    /*─······································································─*/

    void clear( void* address ){
         if( address == nullptr ){ return; }
         memset( address, 0, sizeof(bool) );
    }

public:

   ~poll_t() noexcept { if( obj.count() > 1 ){ return; } close( obj->pd ); }

    poll_t() : obj( new NODE() ) {
        obj->pd = epoll_create1(0); if( obj->pd==-1 )
      { throw except_t("Can't open more epoll fd"); }
        obj->ev.resize( limit::get_soft_fileno() );
    }

    /*─······································································─*/

    void clear() const noexcept { /*--*/ obj->queue.clear(); }

    ulong size() const noexcept { return obj->queue.size (); }

    bool empty() const noexcept { return obj->queue.empty(); }

    /*─······································································─*/

    template< class T, class U, class... W >
    void* add( T& inp, uchar imode, U cb, const W&... args ) noexcept {
    auto addr = push( inp.get_fd(), cb, args... ); /*----------*/
    bool    x = listen( inp.get_fd(), imode, obj->queue.last() );
    if( !x ){ clear( addr ); } return addr; }

    /*─······································································─*/

    int next() noexcept {
        if( obj->blk ){ return 1; }
    coBegin

        if((obj->len=epoll_wait( obj->pd, &obj->ev, obj->ev.size(), 0 ))<=0 )
          { coEnd; } obj->mem.clear();

        do{ auto y=0; while( y < obj->len ){ auto x = obj->ev[y];
            obj->mem.append({ x.data.fd, x.data.ptr });
        ++y; }} while(0);

        coWait( emit()>=0 );

    coFinish }

};}

/*────────────────────────────────────────────────────────────────────────────*/