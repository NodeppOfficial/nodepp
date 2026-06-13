/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#if ( _OS_ == NODEPP_OS_LINUX )
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
    #define NODEPP_HAS_URING 1
#else 
    #define NODEPP_HAS_URING 0
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT_SCHEDULER

#if   ( _OS_ == NODEPP_OS_FRBSD ) || ( _OS_ == NODEPP_OS_APPLE )
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_KQUEUE
#elif ( _OS_ == NODEPP_OS_LINUX ) && ( NODEPP_HAS_URING==1 )
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_IOURING
    #include "uring.h"
#elif ( _OS_ == NODEPP_OS_LINUX ) && ( NODEPP_HAS_URING==0 )
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_EPOLL
#else
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_LITE
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_EVENT_SCHEDULER == NODEPP_SCHEDULER_IOURING
#include "uring.h"

namespace nodepp { class kernel_t {
private:

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
         KV_STATE_OPEN    = 0b10000000,
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00000100,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_CLOSED  = 0b00001000,
         KV_STATE_FALLBACK= 0b00000001
    };

    struct kevent_t { 
        int fd, flag; uchar_64 pd;
        function_t<int> callback;
        event_t   <   > event;
    };

    bool is_std( int fd ) const noexcept { 
        return fd == STDOUT_FILENO ||
               fd == STDIN_FILENO  ||
               fd == STDERR_FILENO ;
    }

protected:

    void* append( kevent_t kv ) const noexcept {

        obj->kv_queue.push( kv );
        auto id = obj->kv_queue.last();

        obj->uring->append( kv.pd, id );

    return (void*) id; }

    int remove( void* ptr ) const noexcept {

        auto kv = obj->kv_queue.as( ptr );
        if ( kv== nullptr ){ return -1; }

        obj->uring  ->remove(kv->data.pd);
        obj->kv_queue.erase (kv); 
    
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

    template< class T >
    void invoker( T* y ) const noexcept { 

        if( !obj->kv_queue.is_valid( y ) ) /**/ { return; }
        if( y->data.flag & FLAG::KV_STATE_USED ){ return; }
            
        y->data.flag |=  FLAG::KV_STATE_USED;
        y->data.event.emit();
        y->data.flag &=~ FLAG::KV_STATE_USED;

    }

protected:

    struct NODE { 
        
        uring_t* uring; int idx, state;
        
        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe   ;
        ulong /*-------*/ timeout ;

    };  ptr_t<NODE> obj;

public:

    kernel_t() : obj( new NODE() ) { 
        obj->uring = &NODEPP_URING();
        obj->state =  KV_STATE_OPEN ; 
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
    
        if( obj->uring->address( inp.get_pd() )==nullptr ){
            obj->uring->append ( &inp );

            kevent_t     kv  ; kv.pd = inp.get_pd();
            kv.flag    = flag; kv.fd = inp.get_fd();
            ulong time = timeout==0 ? 0 : process::now() + timeout;
            
            kv.callback = [=](){
                if( time!=0 && time<process::now() )
                  { inp.close(); /*--*/ return -1; }
                if( inp.is_closed () ){ return -1; } 
                if( inp.is_waiting() ){ return  0; }
            return 1; };

        append(kv); }

        ptr_t<task_t> task( 0UL, task_t() );

        task->addr  = obj->uring->address( inp.get_pd() );
        task->flag  = TASK_STATE::OPEN; 
        task->sign  = &obj;

        obj->kv_queue.as(task->addr)->data.event.add([=](){ 
            return clb( args... ); 
        });

    return task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) const noexcept {
    ptr_t<task_t> tsk = obj->ev_queue.add( args... ); wake(); return tsk; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & FLAG::KV_STATE_SLEEP; }
    int         wake() const noexcept { return obj->uring->wake(); }
    ulong  get_delay() const noexcept { return get_delay_ms(); }

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

    inline int next() const {

        /* EVENT_LOOP EXCECUTION */
        while( obj->ev_queue.next() >= 0 ){ return 1; } 
        set_timeout(obj->ev_queue.get_delay());
        auto stamp = process::now();

        /* CLOSED KILLER */
        do {
        if ( obj->kv_queue.next  () != nullptr ){ auto x = obj->kv_queue.get();
        if ( x->data.event.empty () ){ remove(x); break; }
        if ( x->data.callback()==-1 ){ remove(x); break; } invoker(x); }} while(0);

        /* IO DETECTION */
        obj->state |=  FLAG::KV_STATE_SLEEP;
        auto list   =  obj->uring->next( get_delay_ms() );
        obj->state &=~ FLAG::KV_STATE_SLEEP;

        /* EXCECUTION */
        if ( list.empty() ){ return 1; }
        for( auto x: list ){ invoker( obj->kv_queue.as( (void*)x ) ); }
    
    clear_timeout(); return 1; }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_EVENT_SCHEDULER == NODEPP_SCHEDULER_EPOLL

#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

namespace nodepp { class kernel_t {
private:

    using EPOLLFD = struct epoll_event;
    using ETIMER  = struct timespec;

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
         KV_STATE_OPEN    = 0b10000000,
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00000100,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_CLOSED  = 0b00001000,
         KV_STATE_FALLBACK= 0b00000001
    };

    struct kevent_t { 
        int fd, /*-------*/ flag; 
        function_t<int> callback;
        event_t   <    > event; 
    };

    bool is_std( int fd ) const noexcept { 
        return fd == STDOUT_FILENO ||
               fd == STDIN_FILENO  ||
               fd == STDERR_FILENO ;
    }

protected:

    uchar_64 append( kevent_t kv ) const noexcept {

        if( kv.flag==0x00 || is_std( kv.fd ) ){ return KV_STATE_FALLBACK; }

        obj->kv_queue.push( kv ); auto id = obj->kv_queue.last();

        EPOLLFD event ;

        event.events  = id->data.flag & FLAG::KV_STATE_READ 
                      ? EPOLLIN : EPOLLOUT ;
        event.events |= id->data.flag & FLAG::KV_STATE_EDGE
                      ? EPOLLET : 0x00 ;

        event.data.fd = id->data.fd; event.data.ptr = id;

        if( epoll_ctl( obj->pd, EPOLL_CTL_ADD, id->data.fd, &event )!=0 )
          { obj->kv_queue.erase(id); return KV_STATE_FALLBACK; }
        
    return (uchar_64) id; }

    int remove( void* ptr ) const noexcept {
        auto kv = obj->kv_queue.as( ptr );
        if ( kv== nullptr ){ return -1; }

        auto fd = kv->data.fd;

        EPOLLFD event; event.data.fd=fd; event.events=0;
        epoll_ctl( obj->pd, EPOLL_CTL_DEL, fd, &event );
    
    obj->kv_queue.erase(kv); return 1; }

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

    ptr_t<ETIMER> get_delay_tm() const noexcept {

        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        ulong TIME = tasks==0 ? 0 : get_timeout();

        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ) /*------*/ { return nullptr; }}
        if( obj.count()> 1 && tasks==0 ){ return nullptr; }
          
        ptr_t<ETIMER> ts( 0UL, ETIMER() );
        
        ts->tv_sec  =  TIME / 1000;
        ts->tv_nsec = (TIME % 1000) * 1000000;

    return ts; }

    int get_delay_ms() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ) /*------*/ { return -1; }}
        if( obj.count()> 1 && tasks==0 ){ return -1; }
    return tasks==0 ? 0 : get_timeout(); }

    /*─······································································─*/

    template< class T >
    void invoker( T* y ) const noexcept { 

        if( !obj->kv_queue.is_valid( y ) ) /**/ { return; }
        if( y->data.flag & FLAG::KV_STATE_USED ){ return; }
            
        y->data.flag |=  FLAG::KV_STATE_USED;
        y->data.event.emit();
        y->data.flag &=~ FLAG::KV_STATE_USED;

    }

protected:

    struct NODE {

        int pd=-1, ed=-1; int idx, state;

        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe   ;
        ulong /*-------*/ timeout ; ptr_t<EPOLLFD> ev;

       ~NODE(){ ::close( ed ); ::close( pd ); }
    };  ptr_t<NODE> obj;

public:

   ~kernel_t() noexcept { 
        if( obj.count() > 1 || obj->state & FLAG::KV_STATE_CLOSED )
          { return; } /*----*/ obj->state = FLAG::KV_STATE_CLOSED;
    }

    kernel_t() : obj( new NODE() ) {
        obj->ed = eventfd( 0, EFD_CLOEXEC | EFD_NONBLOCK );
        obj->pd = epoll_create1( EPOLL_CLOEXEC ); 

    if( obj->pd==-1 || obj->ed==-1 )
      { NODEPP_THROW_ERROR("Can't Initialize kernel_t"); }
        obj->ev.resize( NODEPP_MAX_BATCH_SIZE );

        EPOLLFD event;
        event.data.fd  = obj->ed; 
        event.data.ptr = nullptr; 
        event.events   = EPOLLIN;

    if( epoll_ctl( obj->pd, EPOLL_CTL_ADD, obj->ed, &event )==-1 )
      { NODEPP_THROW_ERROR("Can't Initialize kernel_t"); }
      
        obj->state = KV_STATE_OPEN;

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

        if( clb( args... )==-1 ){ return nullptr; }
    
        if( inp.get_pd()==KV_STATE_FALLBACK ){ if( is_std( inp.get_fd() ) ){
            return loop_add( coroutine::add( COROUTINE(){
            coBegin; 

                while( clb( args... )>=0 )
                     { coDelay( 100 ); }

            coFinish
            }));
        } else { return loop_add( cb, args... ); }}

        if( obj->kv_queue.as( (void*) inp.get_pd() )==nullptr ) {

            kevent_t     kv  ;
            kv.flag    = flag; kv.fd = inp.get_fd();
            ulong time = timeout==0 ? 0 : process::now() + timeout;
            
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
            return clb( args... ); 
        });

    return task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) const noexcept {
    ptr_t<task_t> tsk = obj->ev_queue.add( args... ); wake(); return tsk; }

    /*─······································································─*/

    bool  is_sleeping() const noexcept { return obj->state & FLAG::KV_STATE_SLEEP; }

    ulong get_delay  () const noexcept { return get_delay_ms(); }

    int wake() const noexcept { uint64_t value=1; 
    return ::write(obj->ed,&value,sizeof(value)); }

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

    inline int next() const {

        /* EVENT_LOOP EXCECUTION */
        while( obj->ev_queue.next() == 1 ){ return 1; } 
        set_timeout(obj->ev_queue.get_delay());
        auto stamp = process::now();

        /* CLOSED KILLER */
        do {
        if ( obj->kv_queue.next  () != nullptr ){ auto x = obj->kv_queue.get();
        if ( x->data.event.empty () ){ remove(x); break; }
        if ( x->data.callback()==-1 ){ remove(x); break; } invoker(x); }} while(0);

        /* IO DETECTION */
        obj->state |=  FLAG::KV_STATE_SLEEP;

    #if defined( SYS_epoll_pwait2 )

        obj->idx  = ( obj->state & FLAG::KV_STATE_FALLBACK )==0 ? 
        epoll_pwait2( obj->pd, &obj->ev, obj->ev.size(),&get_delay_tm(), nullptr ) :
        epoll_wait  ( obj->pd, &obj->ev, obj->ev.size(), get_delay_ms()) /*-----*/ ;
        
        if( obj->idx==-1 && errno==ENOSYS ) { obj->state |= FLAG::KV_STATE_FALLBACK; }

    #else
        /*----------*/ obj->idx=epoll_wait  ( obj->pd, &obj->ev, obj->ev.size(), get_delay_ms() );
    #endif

        obj->state &=~ FLAG::KV_STATE_SLEEP;

        /* EXCECUTION */
        if   ( obj->idx<= 0 ){ return 1; }
        while( obj->idx > 0 ){ obj->idx--; auto &x = obj->ev[ obj->idx ];

            if( x.data.ptr == nullptr ) { uint64_t value = 0;
            if( ::read( obj->ed, &value, sizeof(uint64_t)) ){ /*unused*/ } continue; }

            auto y=obj->kv_queue.as( x.data.ptr );
            if ( y==nullptr ) /**/ { continue ; } invoker(y);

        }   clear_timeout(); 
    
    return 1; }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_EVENT_SCHEDULER == NODEPP_SCHEDULER_KQUEUE

#include <sys/types.h>
#include <sys/event.h>

namespace nodepp { class kernel_t {
private:

    using KTIMER  = struct timespec;
    using KPOLLFD = struct kevent;

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
         KV_STATE_OPEN    = 0b10000000,
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00000100,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_CLOSED  = 0b00001000,
         KV_STATE_FALLBACK= 0b00000001
    };

    struct kevent_t {
        int fd, /*-------*/ flag; 
        function_t<int> callback;
        event_t   <   > event; 
    };

    bool is_std( int fd ) const noexcept { 
        return fd == STDOUT_FILENO ||
               fd == STDIN_FILENO  ||
               fd == STDERR_FILENO ;
    }

protected:

    uchar_64 append( kevent_t kv ) const noexcept {

        if( kv.flag==0x00 || is_std( kv.fd ) ){ return FLAG::KV_STATE_FALLBACK; }

        obj->kv_queue.push( kv ); auto id = obj->kv_queue.last();

        KPOLLFD event;

        int fd = id->data.fd;
        int fl = id->data.flag & FLAG::KV_STATE_READ 
               ? EVFILT_READ   : EVFILT_WRITE;

        int fc = EV_ADD | EV_ENABLE;
            fc|= id->data.flag & FLAG::KV_STATE_EDGE
               ? EV_CLEAR      : 0x00 ;

        EV_SET( &event, fd, fl, fc, 0, 0, (void*) id );

        if( kevent( obj->pd, &event, 1, NULL, 0, NULL ) !=0 )
          { obj->kv_queue.erase(id); return FLAG::KV_STATE_FALLBACK; }
        
    return (uchar_64) id; }

    int remove( void* ptr ) const noexcept {
        auto kv = obj->kv_queue.as( ptr );
        if ( kv== nullptr ){ return -1; }

        auto fl = kv->data.flag & FLAG::KV_STATE_READ 
                ? EVFILT_READ   : EVFILT_WRITE;
        auto fd = kv->data.fd;

        KPOLLFD event ;
        EV_SET( &event, fd, fl, EV_DELETE, 0, 0, NULL );
        kevent( obj->pd, &event, 1, NULL, 0, NULL );

    obj->kv_queue.erase(kv); return 1; }

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

    ptr_t<KTIMER> get_delay_tm() const noexcept {

        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        ulong TIME = tasks==0 ? 0 : get_timeout();

        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ) /*------*/ { return nullptr; }}
        if( obj.count()> 1 && tasks==0 ){ return nullptr; }
          
        ptr_t<KTIMER> ts( 0UL, KTIMER() );
        
        ts->tv_sec  =  TIME / 1000;
        ts->tv_nsec = (TIME % 1000) * 1000000;

    return ts; }

    int get_delay_ms() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ) /*------*/ { return -1; }}
        if( obj.count()> 1 && tasks==0 ){ return -1; }
    return tasks==0 ? 0 : get_timeout(); }

    /*─······································································─*/

    template< class T >
    void invoker( T* y ) const noexcept { 

        if( !obj->kv_queue.is_valid( y ) ) /**/ { return; }
        if( y->data.flag & FLAG::KV_STATE_USED ){ return; }
            
        y->data.flag |=  FLAG::KV_STATE_USED;
        y->data.event.emit();
        y->data.flag &=~ FLAG::KV_STATE_USED;

    }

protected:

    struct NODE {

        int pd=-1, idx, state;

        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe   ; 
        ulong /*-------*/ timeout ; ptr_t<KPOLLFD> ev;

       ~NODE(){ close( pd ); }
    };  ptr_t<NODE> obj;

public:

   ~kernel_t() noexcept { 
        if( obj.count() > 1 || obj->state & FLAG::KV_STATE_CLOSED )
          { return; } /*----*/ obj->state = FLAG::KV_STATE_CLOSED;
    }

    kernel_t() : obj( new NODE() ) {
        obj->pd = kqueue();

    if( obj->pd==-1 )
      { NODEPP_THROW_ERROR("Can't Initialize kernel_t"); }
        obj->ev.resize( NODEPP_MAX_BATCH_SIZE );

        KPOLLFD ev;
        EV_SET( &ev, 0, EVFILT_USER, EV_ADD, 0, 0, nullptr );
        
    if( kevent( obj->pd, &ev, 1, NULL, 0, NULL ) == -1 )
      { NODEPP_THROW_ERROR("Can't Initialize kernel_t"); }

        obj->state = FLAG::KV_STATE_OPEN;

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
        if( is_std( inp.get_fd() ) ) /*---------*/ {
            return loop_add( coroutine::add( COROUTINE(){
            coBegin; 

                while( clb( args... )>=0 )
                     { coDelay( 100 ); }

            coFinish
            }));
        } else { return loop_add( cb, args... ); }}

        if( obj->kv_queue.as( (void*) inp.get_pd() )==nullptr ) {

            kevent_t     kv  ;
            kv.flag    = flag; kv.fd = inp.get_fd();
            ulong time = timeout==0 ? 0 : process::now() + timeout;
            
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
            return clb( args... ); 
        });

    return task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) const noexcept {
    ptr_t<task_t> tsk = obj->ev_queue.add( args... ); wake(); return tsk; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & FLAG::KV_STATE_SLEEP; }

    ulong get_delay () const noexcept { return get_delay_ms(); }

    int wake() const noexcept {
        KPOLLFD ev;
        EV_SET( &ev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr );
        return kevent( obj->pd, &ev, 1, NULL, 0, NULL );
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

    inline int next() const {

        /* EVENT_LOOP EXCECUTION */
        while( obj->ev_queue.next() >= 0 ){ return 1; } 
        set_timeout(obj->ev_queue.get_delay());
        auto stamp = process::now();

        /* CLOSED KILLER */
        do {
        if ( obj->kv_queue.next  () != nullptr ){ auto x = obj->kv_queue.get();
        if ( x->data.event.empty () ){ remove(x); break; }
        if ( x->data.callback()==-1 ){ remove(x); break; } invoker(x); }} while(0);

        /* IO DETECTION */
        obj->state |=  FLAG::KV_STATE_SLEEP;
        obj->idx=kevent( obj->pd, NULL, 0, &obj->ev, obj->ev.size(), &get_delay_tm() );
        obj->state &=~ FLAG::KV_STATE_SLEEP;

        /* EXCECUTION */
        if   ( obj->idx<= 0 ){ return 1; }
        while( obj->idx > 0 ){ obj->idx--; auto &x = obj->ev[ obj->idx ];
            
            if( x.filter==EVFILT_USER )/*-*/{ continue; }
            auto y = obj->kv_queue.as/*---*/( x.udata ); invoker(y);

        }   clear_timeout();
        
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
         KV_STATE_USED    = 0b00000100,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_CLOSED  = 0b00001000,
         KV_STATE_FALLBACK= 0b00000001
    };

    struct kevent_t { function_t<int> callback; int fd, flag; };

    bool is_std( int fd ) const noexcept { 
    return fd == STDOUT_FILENO ||
           fd == STDIN_FILENO  ||
           fd == STDERR_FILENO ;
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
    ptr_t<task_t> poll_add( T& inp, int flag, U cb, ulong timeout=0, const W&... args ) const noexcept {

        function_t<int,W...> clb ( cb ); if ( inp.is_closed() ) { return nullptr; }
        auto time = type::bind( timeout>0 ? timeout + process::now() : timeout );

        if( clb( args... )==-1 ){ return nullptr; }
        
        return loop_add( coroutine::add( COROUTINE(){
        coBegin 

            if( *time > 0 && *time < process::now() ){ coEnd; }
            if( is_std( inp.get_fd() ) ) /**/ { coDelay(100); }

            coSet(0); return clb( args... ) >= 0 ? 1 : -1; 

        coFinish
        }));

    }

    template< class T, class... V >
    ptr_t<task_t> loop_add ( T cb, const V&... args ) const noexcept {
        return obj->ev_queue.add( cb, args... );
    }

    /*─······································································─*/

    inline int next() const {

        while( obj->ev_queue.next() >= 0 ){ return 1; } 

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

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/