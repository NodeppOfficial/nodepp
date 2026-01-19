/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_KERNEL
#define NODEPP_WINDOWS_KERNEL
#if( _OS_ == NODEPP_OS_WINDOWS )
    #define NODEPP_POLL_IOCP
#else
    #define NODEPP_POLL_NPOLL
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_POLL_IOCP

#include <winsock2.h>
#include <mswsock.h>
#include "../loop.h"

namespace nodepp { class kernel_t: public generator_t {
private:

    using HPOLLFD = OVERLAPPED_ENTRY;

    enum FLAG { 
         KV_STATE_UNKNONW = 0b00000000, 
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00000100,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_CLOSED  = 0b00001000
    };

    struct kevent_t { public:
        function_t<int> callback;
        ulong timeout; HANDLE fd; int flag; 
    };

    bool is_std( HANDLE fd ) const noexcept { 
        return fd == GetStdHandle( STD_INPUT_HANDLE ) ||
               fd == GetStdHandle( STD_OUTPUT_HANDLE) ||
               fd == GetStdHandle( STD_ERROR_HANDLE ) ;
    }

protected:

    void* append( kevent_t kv ) const noexcept {

        if( kv.flag==0x00 || is_std( kv.fd ) ){ return nullptr; }

        auto tm = obj->kv_queue.as( get_nearest_timeout( kv.timeout ) );
        /*-----*/ obj->kv_queue.insert( tm, kv );
        auto id = tm==nullptr ? obj->kv_queue.last(): tm->prev;

        if( !CreateIoCompletionPort( id->data.fd, obj->pd, (ULONG_PTR)id, 0 ) ) 
          { obj->kv_queue.erase(id); return nullptr; }

    iocp_execute_callback( id ); return (void*)id; }

    int remove( void* ptr ) const noexcept { 
        
        if( ptr == nullptr ){ return -1; } 
        auto kv = obj->kv_queue.as( ptr ); 

        kv->data.flag = FLAG::KV_STATE_CLOSED;
        obj->kv_queue.erase( kv );
        
    return 0; }

    /*─······································································─*/

    void* get_nearest_timeout( ulong time ) const noexcept {
        if( time == 0 ){ return nullptr; }

        auto x = obj->kv_queue.first(); while( x!=nullptr ){
        if( 0   ==x->data.timeout ){ return x; }
        if( time<=x->data.timeout ){ return x; }
        x = x->next; }

    return nullptr; }

    /*─······································································─*/

    void iocp_execute_callback( void* address ) const noexcept {

        if( address == nullptr ) { return; }
        auto y = obj->kv_queue.as( address );

        if( y->data.flag & FLAG::KV_STATE_USED ){ return; }
            y->data.flag|= FLAG::KV_STATE_USED;

        obj->ev_queue.add( coroutine::add( COROUTINE(){
        coBegin

            do { switch( y->data.callback() ) {
            case -1: y->data.flag &=~ FLAG::KV_STATE_USED; remove(y); coEnd; break;
            case  0: y->data.flag &=~ FLAG::KV_STATE_USED; /*------*/ coEnd; break;
            case  1: break; } coNext; } while(1);

        coFinish
        }));

    }

    /*─······································································─*/

    int get_delay() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        ulong time = TIMEOUT; time = time == 0 ? 1: time;
        return ( tasks==0 && obj->kv_queue.size()>0 )?-1: 
               ( tasks==0 && obj.count()         >1 )?-1: time;
    }

protected:

    struct NODE {
        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe;
        ptr_t<HPOLLFD>    ev;
        HANDLE pd; ULONG idx;
    };  ptr_t<NODE> obj;

public:

    kernel_t() : obj( new NODE() ) {
        obj->pd = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
        if( obj->pd == NULL ){ throw except_t("Can't Initializate kernel_t"); }
        obj->ev.resize( MAX_PATH );
    }

   ~kernel_t() noexcept { 
        if( obj.count() > 1 ) return; 
        CloseHandle( obj->pd ); 
    }

public:

    ulong size() const noexcept { return obj->ev_queue.size() + obj->kv_queue.size() + obj->probe.get() + obj.count()-1; }

    void clear() const noexcept { /*--*/ obj->ev_queue.clear(); obj->kv_queue.clear(); obj->probe.clear(); }

    bool empty() const noexcept { return size()==0; }

    /*─······································································─*/

    void off( ptr_t<task_t> address ) const noexcept { clear( address ); }

    void clear( ptr_t<task_t> address ) const noexcept {
        if( address.null() ) /*-*/ { return; }
        if( address->sign == &obj ){
        if( address->flag & TASK_STATE::CLOSED ){ return; }
            address->flag = TASK_STATE::CLOSED;
            remove( address->addr ); 
        } else { obj->ev_queue.off( address ); }
    }

    /*─······································································─*/

    template< class T, class U, class... W >
    ptr_t<task_t> poll_add( T& inp, int flag, U cb, ulong timeout=0, const W&... args ) noexcept {
    ptr_t<task_t> task( 0UL, task_t() ); auto clb = type::bind( cb );

        kevent_t       kv; 
        kv.flag      = flag;
        kv.fd        = (HANDLE) inp.get_fd();
        kv.timeout   = timeout==0 ? 0 : process::now() + timeout;
        
        kv.callback  = [=](){ 
            int c=(*clb)( args... );
            if( inp.is_closed () ){ return -1; } 
            if( inp.is_waiting() ){ return  0; }
        return c; };

        task->addr   = append( kv ); 
        task->sign   = &obj;

    return task->addr==nullptr ? loop_add( cb, args... ) : task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) noexcept {
        return obj->ev_queue.add( args... );
    }

    /*─······································································─*/

    int emit() const noexcept {
        return PostQueuedCompletionStatus( obj->pd, 0, 0, NULL ) ? 1 : -1;
    }

    /*─······································································─*/

    template< class T, class... V > 
    int await( T cb, const V&... args ){ 
    int c=0; probe_t tmp = obj->probe;

        if ((c =cb(args...))>=0 ){
        if ( c==1 ){ auto t = coroutine::getno().delay;
        if ( t >0 ){ process::set_timeout( t ); }
        else /*-*/ { process::set_timeout(0UL); }} next(); return 1; } 
    
    return -1; }

    /*─······································································─*/

    inline int next() noexcept {
    coBegin

        coWait( obj->ev_queue.next()>=0 ); process::set_timeout( obj->ev_queue.get_delay() );

        do   { auto x=obj->kv_queue.first(); while( x != nullptr ){
               auto y=x->next; if( x->data.timeout==0 ){ break; }
        if   ( x->data.timeout < process::now() ){ remove(x); }
        else { break; } x=y; }} while(0);

        GetQueuedCompletionStatusEx( obj->pd, obj->ev.data(), obj->ev.size(), 
        /*----------------------*/ & obj->idx, get_delay(), FALSE );

        while( obj->idx!=0 ){ do { obj->idx--;
        if( obj->ev[ obj->idx ].lpCompletionKey == (ULONG_PTR)NULL ){ break; }

            auto x = (void*) obj->ev[ obj->idx ].lpCompletionKey;
            auto y = obj->kv_queue.as  ( x );
            if( !obj->kv_queue.is_valid( y ) ){ break; }

            iocp_execute_callback( x );

        } while(0); coNext; } process::clear_timeout();

    coFinish }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_POLL_NPOLL

namespace nodepp { class kernel_t: public generator_t {
private:

    enum FLAG { 
         KV_STATE_UNKNONW = 0b00000000, 
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00000100,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_CLOSED  = 0b00001000
    };

    struct kevent_t { public:
        function_t<int> callback;
        ulong timeout; int fd, flag; 
    };

    /*─······································································─*/

    int get_delay() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        ulong time = TIMEOUT; /*-*/ time = time == 0  ?  10: time;
        return ( tasks==0 && obj.count()         > 1 )? 100: time;
    }

protected:

    struct NODE {
        probe_t   probe;
        loop_t ev_queue;
    };  ptr_t<NODE> obj;

public:

    kernel_t() noexcept : obj( new NODE() ) {}

public:

    void off( ptr_t<task_t> address ) const noexcept { clear( address ); }

    void clear( ptr_t<task_t> address ) const noexcept {
         if( address.null() ) /*--------------*/ { return; }
         if( address->flag & TASK_STATE::CLOSED ){ return; }
             address->flag = TASK_STATE::CLOSED;
    }

    /*─······································································─*/

    ulong size() const noexcept { return obj->ev_queue.size() + obj->probe.get() + obj.count()-1; }

    void clear() const noexcept { /*--*/ obj->ev_queue.clear(); obj->probe.clear(); }

    bool empty() const noexcept { return size()==0; }

    int   emit() const noexcept { return -1; }

    /*─······································································─*/

    template< class T, class U, class... W >
    ptr_t<task_t> poll_add ( T /*unused*/, int /*unused*/, U cb, ulong timeout=0, const W&... args ) noexcept {
        auto time = type::bind( timeout>0 ? timeout + process::now() : timeout );
        auto clb  = type::bind( cb ); return obj->ev_queue.add( [=](){ 
        if( *time > 0 && *time < process::now() ){ return -1; }
            return (*clb)( args... )>=0 ? 1 : -1; 
        } );
    }

    template< class T, class... V >
    ptr_t<task_t> loop_add ( T cb, const V&... args ) noexcept {
        return obj->ev_queue.add( cb, args... );
    }

    /*─······································································─*/

    template< class T, class... V > 
    int await( T cb, const V&... args ){ 
    int c=0; probe_t tmp = obj->probe;

        if ((c =cb(args...))>=0 ){
        if ( c==1 ){ auto t = coroutine::getno().delay;
        if ( t >0 ){ process::set_timeout( t ); }
        else /*-*/ { process::set_timeout(0UL); }} next(); return 1; } 
    
    return -1; }

    /*─······································································─*/

    inline int next() noexcept {
    coBegin

        coWait( obj->ev_queue.next()>=0 );

        process::set_timeout( obj->ev_queue.get_delay() );
        process::delay( get_delay() );
        process::clear_timeout();

    coFinish }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/