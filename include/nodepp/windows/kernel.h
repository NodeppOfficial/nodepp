/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT_SCHEDULER

#if   ( _OS_ == NODEPP_OS_WINDOWS )
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_IOCP
#else
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_LITE
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_EVENT_SCHEDULER == NODEPP_SCHEDULER_IOCP

/*────────────────────────────────────────────────────────────────────────────*/

#include <winsock2.h>
#include <mswsock.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class kernel_t {
private:

    using HPOLLFD = OVERLAPPED_ENTRY;

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
         KV_STATE_OPEN    = 0b10000000,
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00100000,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_CLOSED  = 0b00001000,
         KV_STATE_FALLBACK= 0b00000001
    };

    struct kevent_t { 
        int flag; /**/ HANDLE fd;
        function_t<int> callback; 
        event_t   <   > event   ;
    };

    bool is_std( HANDLE fd ) const noexcept { 
        return fd == GetStdHandle( STD_INPUT_HANDLE ) ||
               fd == GetStdHandle( STD_OUTPUT_HANDLE) ||
               fd == GetStdHandle( STD_ERROR_HANDLE ) ;
    }

protected:

    uchar_64 append( kevent_t kv ) const noexcept {

        obj->kv_queue.push( kv ); auto id = obj->kv_queue.last();

        if( !CreateIoCompletionPort( id->data.fd, obj->pd, (ULONG_PTR)id, 0 ) ) 
          { obj->kv_queue.erase(id); return FLAG::KV_STATE_FALLBACK; }

    return (uchar_64) id; }

    int remove( void* ptr ) const noexcept {

        auto kv = obj->kv_queue.as( ptr );
        if ( kv== nullptr ){ return -1; }
        
        obj->kv_queue.erase( kv );
        
    return 1; }

    /*─······································································─*/

    void clear_timeout() const noexcept { get_timeout(true); }

    ulong set_timeout( int time=0 ) const noexcept { 
        if( time < 0 ){ /*--------------*/ return 1; }
        auto stamp=&get_timeout(); ulong out=*stamp;
        if( *stamp>(ulong)time ){ *stamp=(ulong)time; }
    return out; }

    ulong& get_timeout( bool reset=false ) const noexcept {
        if( reset ) { obj->timeout=(ulong)-1; }
    return obj->timeout; }
    
    /*─······································································─*/
    
    int get_delay_ms() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ) /*------*/ { return -1; }}
        if( obj.count()> 1 && tasks==0 ){ return -1; }
    return tasks==0 ? 0 : get_timeout(); }

    void invoker( void* address ) const noexcept {
    if( address == nullptr ){ do {

        auto x = obj->kv_queue.get();

        if( x == nullptr ) /*----*/ { /*------*/ return; }
        if( x->data.event.empty()  ){ remove(x); return; }
        if( x->data.callback()==-1 ){ remove(x); return; }
        if( x->data.flag & FLAG::KV_STATE_USED ){ break; }
            
            x->data.flag|= FLAG::KV_STATE_USED;
            x->data.event.emit(); 
            x->data.flag&=~FLAG::KV_STATE_USED;

    } while(0); obj->kv_queue.next(); } else { do {

        auto x = obj->kv_queue.as( address );

        if( x == nullptr ) /*----------------*/ { return; }
        if( x->data.flag & FLAG::KV_STATE_USED ){ return; }
            x->data.flag|= FLAG::KV_STATE_USED;

        auto self = type::bind( this );
        
        loop_add( coroutine::add( COROUTINE(){
        coBegin
        
            do{ if( !self->obj->kv_queue.is_valid(x) ){ return -1; }
                x->data.event.emit(); switch( x->data.callback() ) {
                case -1: self->remove(x); /*--------------*/ return -1; break;
                case  0: x->data.flag&=~FLAG::KV_STATE_USED; return -1; break;
                case  1: /*-------------------------------*/ return  1; break; 
            }} while(0);

        coFinish
        })); 
    
    } while(0); }}

protected:

    struct NODE {

        HANDLE pd; ULONG idx; int state;

        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe   ; 
        ulong /*-------*/ timeout ; ptr_t<HPOLLFD> ev;

       ~NODE(){ CloseHandle( pd ); }
    };  ptr_t<NODE> obj;

public:

    kernel_t() : obj( new NODE() ) {
        obj->pd = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
        if( obj->pd == NULL ){ NODEPP_THROW_ERROR("Can't Initialize kernel_t"); }
        obj->ev.resize( NODEPP_MAX_BATCH_SIZE );
    }

   ~kernel_t() noexcept { 
        if( obj.count() > 1 || obj->state & FLAG::KV_STATE_CLOSED )
          { return; } /*----*/ obj->state = FLAG::KV_STATE_CLOSED;
    }

public:

    ulong size() const noexcept { return obj->ev_queue.size() + obj->kv_queue.size() + obj->probe.get() + obj.count()-1; }

    void clear() const noexcept { /*--*/ obj->ev_queue.clear(); obj->kv_queue.clear(); obj->probe.clear(); }
    
    bool should_close() const noexcept { return empty() || NODEPP_SHTDWN() || NODEPP_LOCAL_SHTDWN(); }

    bool empty() const noexcept { return size()==0; }

    /*─······································································─*/

    void off  ( ptr_t<task_t> address ) const noexcept { clear( address ); }

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
    ptr_t<task_t> poll_add( T& inp, int flag, U cb, ulong timeout=0, const W&... args ) const noexcept {
    function_t<int,W...> clb ( cb ); if( inp.is_closed() ){ return nullptr; }

        if( inp.get_pd()==FLAG::KV_STATE_FALLBACK ){ 
        if( is_std( (HANDLE) inp.get_fd() ) ) /**/ {
            return loop_add( coroutine::add( COROUTINE(){
            coBegin; 

                while( clb( args... )>=0 )
                     { coDelay( 100 ); }

            coFinish
            }));
        } else { return loop_add( cb, args... ); }}

        if( obj->kv_queue.as( (void*) inp.get_pd() )==nullptr ) {

            kevent_t     kv  ;
            kv.flag    = flag; kv.fd = (HANDLE) inp.get_fd();
            len_t time = timeout==0 ? 0 : process::now() + timeout;
            
            kv.callback = [=](){
                if( time!=0 && time<process::now() )
                  { inp.close(); /*--*/ return -1; }
                if( inp.is_closed () ){ return -1; } 
                if( inp.is_waiting() ){ return  0; }
            return 1; };

            inp.get_pd() = append(kv);

            if( inp.get_pd()==KV_STATE_FALLBACK )
              { return poll_add( inp, flag, cb, timeout, args... ); }

        }

        ptr_t<task_t> task( 0UL, task_t() );
        task->addr  = (void*) inp.get_pd();
        task->flag  = TASK_STATE::OPEN; 
        task->sign  = &obj;
        
        obj->kv_queue.as(task->addr)->data.event.add([=](){ 
            return clb( args... )>=0 ? 1 : -1; 
        }); invoker( task->addr );

    return task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) const noexcept {
    ptr_t<task_t> tsk = obj->ev_queue.add( args... ); wake(); return tsk; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & FLAG::KV_STATE_SLEEP; }

    ulong get_delay () const noexcept { return get_delay_ms(); }

    int wake() const noexcept {
        return PostQueuedCompletionStatus( obj->pd, 0, 0, NULL ) ? 1 : -1;
    }

    /*─······································································─*/

    template< class T, class... V > 
    int await( T cb, const V&... args ) const { 
    int c=0; probe_t tmp = obj->probe;

        if ((c =cb(args...))>=0 ){
        if ( c==1 ){ auto t = coroutine::getno().delay;
        if ( t >0 ){ set_timeout( t ); }
        else /*-*/ { set_timeout(0UL); }} next(); return 1; } 
    
    return -1; }

    /*─······································································─*/

    int next() const { invoker( nullptr );

        if( obj->ev_queue.next()>=0 ){ return 1; } 
        set_timeout(obj->ev_queue.get_delay());
        
        obj->state |= FLAG::KV_STATE_SLEEP;

        int res= GetQueuedCompletionStatusEx( 
            obj->pd , obj->ev.data(), obj->ev.size(), 
           &obj->idx, get_delay_ms(), FALSE
        );

        obj->state &=~ FLAG::KV_STATE_SLEEP; 

        if( res ){ while( obj->idx > 0 ){ obj->idx--; auto &x = obj->ev[ obj->idx ];

            if( x.lpCompletionKey==(ULONG_PTR)NULL ){ continue; }
            invoker( (void*) x.lpCompletionKey );

        } clear_timeout(); } 
        
    return 1; }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_EVENT_SCHEDULER == NODEPP_SCHEDULER_LITE

namespace nodepp { class kernel_t {
private:

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
         KV_STATE_OPEN    = 0b10000000,
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00100000,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_CLOSED  = 0b00001000,
         KV_STATE_FALLBACK= 0b00000001
    };

    struct kevent_t { int fd, flag; function_t<int> callback;  };

    bool is_std( HANDLE fd ) const noexcept { 
        return fd == GetStdHandle( STD_INPUT_HANDLE ) ||
               fd == GetStdHandle( STD_OUTPUT_HANDLE) ||
               fd == GetStdHandle( STD_ERROR_HANDLE ) ;
    }

protected:

    void clear_timeout() const noexcept { get_timeout(true); }

    ulong set_timeout( int time=0 ) const noexcept { 
        if( time < 0 ){ /*--------------*/ return 1; }
        auto stamp=&get_timeout(); ulong out=*stamp;
        if( *stamp>(ulong)time ){ *stamp=(ulong)time; }
    return out; }

    ulong& get_timeout( bool reset=false ) const noexcept {
        if( reset ) { obj->timeout=(ulong)-1; }
    return obj->timeout; }

    int get_delay_ms() const noexcept {
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        if(tasks==0 && obj.count()>1 ){ return 1000; }
    return tasks==0 ? 0 : get_timeout(); }

protected:

    struct NODE {
        int /*-*/ state;
        ulong   timeout;
        probe_t   probe;
        loop_t ev_queue;
    };  ptr_t<NODE> obj;

public:

    kernel_t() noexcept : obj( new NODE() ) {}

public:

    void off  ( ptr_t<task_t> address ) const noexcept { clear( address ); }

    void clear( ptr_t<task_t> address ) const noexcept {
         if( address.null() ) /*--------------*/ { return; }
         if( address->flag & TASK_STATE::CLOSED ){ return; }
             address->flag = TASK_STATE::CLOSED;
    }

    /*─······································································─*/
    
    bool should_close() const noexcept { return empty() || NODEPP_SHTDWN() || NODEPP_LOCAL_SHTDWN(); }

    ulong size() const noexcept { return obj->ev_queue.size() + obj->probe.get() + obj.count()-1; }

    void clear() const noexcept { /*--*/ obj->ev_queue.clear(); obj->probe.clear(); }

    bool empty() const noexcept { return size()==0; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & FLAG::KV_STATE_SLEEP; }

    ulong  get_delay() const noexcept { return get_delay_ms(); }

    int /*---*/ wake() const noexcept { return -1; }

    /*─······································································─*/

    template< class T, class U, class... W >
    ptr_t<task_t> poll_add ( T& inp, int flag, U cb, ulong timeout=0, const W&... args ) const noexcept {

        function_t<int,W...> clb ( cb ); if( inp.is_closed() ) { return nullptr; }
        auto time = type::bind( timeout>0 ? timeout + process::now() : timeout );

        if( clb( args... )==-1 ){ return nullptr; }
        
        return loop_add( coroutine::add( COROUTINE(){
        coBegin 

            if( *time > 0 && *time < process::now() ){ coEnd; }
            if( is_std((HANDLE)inp.get_fd()) ){ coDelay(100); }

            coSet(0); return clb( args... ) >= 0 ? 1 : -1; 

        coFinish
        }));

    }

    template< class T, class... V >
    ptr_t<task_t> loop_add ( T cb, const V&... args ) const noexcept {
        return obj->ev_queue.add( cb, args... );
    }

    /*─······································································─*/

    int next() const {

        if( obj->ev_queue.next()>=0 ){ return 1; } 

        obj->state |=  FLAG::KV_STATE_SLEEP;

        set_timeout(obj->ev_queue.get_delay());
        process::delay( get_delay_ms() );
        clear_timeout();

        obj->state &=~ FLAG::KV_STATE_SLEEP;

    return 1; }

    /*─······································································─*/

    template< class T, class... V > 
    int await( T cb, const V&... args ) const { 
    int c=0; probe_t tmp = obj->probe;

        if ((c =cb(args...))>=0 ){
        if ( c==1 ){ auto t = coroutine::getno().delay;
        if ( t >0 ){ set_timeout( t ); }
        else /*-*/ { set_timeout(0UL); }} next(); return 1; } 
    
    return -1; }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/