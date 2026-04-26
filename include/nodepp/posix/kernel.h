/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_KERNEL
#define NODEPP_POSIX_KERNEL

#if NODEPP_ALLOW_KERNEL_BASED_POLL==1
    #if ( _OS_ == NODEPP_OS_FRBSD ) || ( _OS_ == NODEPP_OS_APPLE )
    #define NODEPP_POLL_KPOLL
    #elif _OS_ == NODEPP_OS_LINUX
    #define NODEPP_POLL_EPOLL
    #else
    #define NODEPP_POLL_NPOLL
    #endif
#else
    #define NODEPP_POLL_NPOLL
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_POLL_EPOLL

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
         KV_STATE_CLOSED  = 0b00001000
    };

    struct kevent_t { public:
        function_t<int> callback;
        ulong timeout; int fd, flag; 
    };

    bool is_std( int fd ) const noexcept { 
        return fd == STDOUT_FILENO ||
               fd == STDIN_FILENO  ||
               fd == STDERR_FILENO ;
    }

protected:

    void* append( kevent_t kv ) const noexcept {

        if( kv.flag==0x00 || is_std( kv.fd ) ){ return nullptr; }

        auto tm = obj->kv_queue.as( get_nearest_timeout( kv.timeout ) );
        /*-----*/ obj->kv_queue.insert( tm, kv );
        auto id = tm==nullptr ? obj->kv_queue.last(): tm->prev;

        EPOLLFD event;

        event.events   = id->data.flag & FLAG::KV_STATE_READ 
                       ? EPOLLIN : EPOLLOUT ;
        event.events  |= id->data.flag & FLAG::KV_STATE_EDGE
                       ? EPOLLET : 0x00 ;
        event.data.fd  = id->data.fd;
        event.data.ptr = id;

        if( epoll_ctl( obj->pd, EPOLL_CTL_ADD, id->data.fd, &event )!=0 )
          { obj->kv_queue.erase(id); return nullptr; }
        
    return (void*) id; }

    int remove( void* ptr ) const noexcept {
        if( ptr == nullptr ){ return -1; }

        auto kv = obj->kv_queue.as( ptr );
        auto fl = kv->data.flag;
        auto fd = kv->data.fd;

        EPOLLFD event; event.data.fd=fd; event.events=0;
        epoll_ctl( obj->pd, EPOLL_CTL_DEL, fd, &event );
    
    obj->kv_queue.erase(kv); return 1; }

    /*─······································································─*/

    bool batch() const noexcept { 
         obj->batch = min( obj->batch, (uchar) NODEPP_MAX_BATCH_SIZE );
         return obj->probe.get()>0 ? obj->batch--!=0 : true; 
    }

    void* get_nearest_timeout( ulong time ) const noexcept {
        if( time == 0 ){ return nullptr; }

        auto x = obj->kv_queue.first(); while( x!=nullptr ){
        if( 0   ==x->data.timeout ){ return x; }
        if( time<=x->data.timeout ){ return x; }
        x = x->next; }

    return nullptr; }

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
        if( obj.count()==1 ){ return nullptr; }}
        if( obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()> 1 ){ return nullptr; }}
          
        ptr_t<ETIMER> ts( 0UL, ETIMER() );
        
        ts->tv_sec  =  TIME / 1000;
        ts->tv_nsec = (TIME % 1000) * 1000000;

    return ts; }

    int get_delay_ms() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ){ return -1; }}
        if( obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()> 1 ){ return -1; }}
    return tasks==0 ? 0 : get_timeout(); }

protected:

    struct NODE {

        int pd=-1, ed=-1; int idx, state; bool pl=true; 
        uchar batch= NODEPP_MAX_BATCH_SIZE;
        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe;
        ulong /*--*/ timeout; 
        ptr_t<EPOLLFD>    ev;

       ~NODE(){ ::close( ed ); ::close( pd ); }
    };  ptr_t<NODE> obj;

public:

   ~kernel_t() noexcept { 
        if( obj.count() > 1 || obj->state & KV_STATE_CLOSED )
          { return; } /*----*/ obj->state = KV_STATE_CLOSED;
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
    
    bool should_close() const noexcept { return empty() || NODEPP_SHTDWN(); }

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
    ptr_t<task_t> poll_add( T& inp, int flag, U cb, ulong timeout=0, const W&... args ) const noexcept {
        if( cb( args... )==-1 ){ return nullptr; }
    
        kevent_t      kv;
        kv.flag     = flag;
        kv.fd       = inp.get_fd(); auto clb = type::bind( cb );
        kv.timeout  = timeout==0 ? 0 : process::now() + timeout;
        
        kv.callback = [=](){ int c=(*clb)( args... );
            if( inp.is_closed () ){ return -1; } 
            if( inp.is_waiting() ){ return  0; }
        return c; };

        ptr_t<task_t> task( 0UL, task_t() );
        task->flag  = TASK_STATE::OPEN;
        task->addr  = append( kv ); 
        task->sign  = &obj;

        if( task->addr==nullptr ){ if( is_std( kv.fd ) ){  

            return loop_add( coroutine::add( COROUTINE(){
            coBegin; 

                while( (*clb)( args... )>=0 )
                     { coDelay( 100 ); }

            coFinish
            }));

        } else { return loop_add( cb, args... ); }}

    wake(); return task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) const noexcept {
    ptr_t<task_t> tsk = obj->ev_queue.add( args... ); wake(); return tsk; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & KV_STATE_SLEEP; }

    ulong get_delay() const noexcept { return get_delay_ms(); }

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
        while( obj->ev_queue.next() >= 0 && batch() ){ return 1; } 
        set_timeout(obj->ev_queue.get_delay());
        auto stamp = process::now();

        /* TIMEOUT KILLER */
        do   { auto x=obj->kv_queue.first(); while( x != nullptr ){
               auto y=x->next; if( x->data.timeout==0 ) { break; }
        if   ( x->data.flag & TASK_STATE::USED ){ x=y; continue; }
        if   ( x->data.timeout < stamp ) /*--*/ { remove(x); }
        else { break; } x=y; }} while(0);

        /* CLOSED KILLER */
        if   ( obj->kv_queue.next() != nullptr ){ 
        if   ( obj->kv_queue.get ()->data.callback()==-1 )
             { remove( obj->kv_queue.get() );  }}

        /* IO DETECTION */
        obj->state |=  KV_STATE_SLEEP;

    #if   defined( SYS_epoll_pwait2 )
        if( obj->pl ){ obj->idx=epoll_pwait2( obj->pd, &obj->ev, obj->ev.size(),&get_delay_tm(), nullptr ); }
        if( obj->idx==-1 && errno==ENOSYS ) { obj->pl = false; }
        if(!obj->pl ){ obj->idx=epoll_wait  ( obj->pd, &obj->ev, obj->ev.size(), get_delay_ms() ); }
    #else
        /*----------*/ obj->idx=epoll_wait  ( obj->pd, &obj->ev, obj->ev.size(), get_delay_ms() );
    #endif

        obj->state &=~ KV_STATE_SLEEP;

        /* EXCECUTION */
    while( obj->idx > 0 ){ obj->idx--; auto x = obj->ev[ obj->idx ];

        if( x.data.ptr==nullptr ){ uint64_t value=0; int c=0;
            c=::read( obj->ed,&value, sizeof(value));
        continue; }

        auto y = obj->kv_queue.as( x.data.ptr );
        if( !obj->kv_queue.is_valid( y ) ){ continue; }

        if( x.events& ( EPOLLERR | EPOLLHUP ) &&
          ( x.events& ( EPOLLOUT | EPOLLIN ))==0
        ) { remove( y ); continue; }

        if( y->data.flag & FLAG::KV_STATE_USED ){ continue; }
            y->data.flag|= FLAG::KV_STATE_USED;

        obj->ev_queue.add( coroutine::add( COROUTINE(){
        coBegin

            do { switch( y->data.callback() ) {
            case -1: remove(y); /*----------------------*/ coEnd; break;
            case  0: y->data.flag &=~ FLAG::KV_STATE_USED; coEnd; break;
            case  1: /*------------------*/ break; } coNext; } while(1);

        coFinish
        }));

    }   clear_timeout(); obj->batch = NODEPP_MAX_BATCH_SIZE; return 1; }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_POLL_KPOLL

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
         KV_STATE_CLOSED  = 0b00001000
    };

    struct kevent_t { public:
        function_t<int> callback;
        ulong timeout; int fd, flag; 
    };

    bool is_std( int fd ) const noexcept { 
        return fd == STDOUT_FILENO ||
               fd == STDIN_FILENO  ||
               fd == STDERR_FILENO ;
    }

protected:

    void* append( kevent_t kv ) const noexcept {

        if( kv.flag==0x00 || is_std( kv.fd ) ){ return nullptr; }

        auto tm = obj->kv_queue.as( get_nearest_timeout( kv.timeout ) );
        /*-----*/ obj->kv_queue.insert( tm, kv );
        auto id = tm==nullptr ? obj->kv_queue.last(): tm->prev;

        KPOLLFD event;

        int fd = id->data.fd;
        int fl = id->data.flag & FLAG::KV_STATE_READ 
               ? EVFILT_READ   : EVFILT_WRITE;

        int fc = EV_ADD | EV_ENABLE;
            fc|= id->data.flag & FLAG::KV_STATE_EDGE
               ? EV_CLEAR : 0x00 ;

        EV_SET( &event, fd, fl, fc, 0, 0, (void*) id );

        if( kevent( obj->pd, &event, 1, NULL, 0, NULL ) !=0 )
          { obj->kv_queue.erase(id); /*--*/ return nullptr; }
        
    return (void*) id; }

    int remove( void* ptr ) const noexcept {
        if( ptr == nullptr ){ return -1; }

        auto kv = obj->kv_queue.as( ptr );
        auto fl = kv->data.flag & FLAG::KV_STATE_READ 
                ? EVFILT_READ   : EVFILT_WRITE;
        auto fd = kv->data.fd;

        KPOLLFD event ;
        EV_SET( &event, fd, fl, EV_DELETE, 0, 0, NULL );
        kevent( obj->pd, &event, 1, NULL, 0, NULL );

    obj->kv_queue.erase(kv); return 1; }

    /*─······································································─*/

    bool batch() const noexcept { 
         obj->batch = min( obj->batch, (uchar) NODEPP_MAX_BATCH_SIZE );
         return obj->probe.get()>0 ? obj->batch--!=0 : true; 
    }

    void* get_nearest_timeout( ulong time ) const noexcept {
        if( time == 0 ){ return nullptr; }

        auto x = obj->kv_queue.first(); while( x!=nullptr ){
        if( 0   ==x->data.timeout ){ return x; }
        if( time<=x->data.timeout ){ return x; }
        x = x->next; }

    return nullptr; }

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
        if( obj.count()==1 ){ return nullptr; }}
        if( obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()> 1 ){ return nullptr; }}
          
        ptr_t<KTIMER> ts( 0UL, KTIMER() );
        
        ts->tv_sec  =  TIME / 1000;
        ts->tv_nsec = (TIME % 1000) * 1000000;

    return ts; }

    int get_delay_ms() const noexcept { 
        ulong tasks= obj->ev_queue.size() + obj->probe.get();
        if(!obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()==1 ){ return -1; }}
        if( obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()> 1 ){ return -1; }}
    return tasks==0 ? 0 : get_timeout(); }

protected:

    struct NODE {

        ulong /*--*/ timeout; int pd, idx , state;
        uchar batch= NODEPP_MAX_BATCH_SIZE;
        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe;
        ptr_t<KPOLLFD>    ev;

       ~NODE(){ close( pd ); }
    };  ptr_t<NODE> obj;

public:

   ~kernel_t() noexcept { 
        if( obj.count() > 1 || obj->state & KV_STATE_CLOSED )
          { return; } /*----*/ obj->state = KV_STATE_CLOSED;
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

        obj->state = KV_STATE_OPEN;

    }

public:

    ulong size() const noexcept { return obj->ev_queue.size() + obj->kv_queue.size() + obj->probe.get() + obj.count()-1; }

    void clear() const noexcept { /*--*/ obj->ev_queue.clear(); obj->kv_queue.clear(); obj->probe.clear(); }
    
    bool should_close() const noexcept { return empty() || NODEPP_SHTDWN(); }

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
    ptr_t<task_t> poll_add( T& inp, int flag, U cb, ulong timeout=0, const W&... args ) const noexcept {
        if( cb( args... )==-1 ){ return nullptr; }
    
        kevent_t      kv;
        kv.flag     = flag;
        kv.fd       = inp.get_fd(); auto clb = type::bind( cb );
        kv.timeout  = timeout==0 ? 0 : process::now() + timeout;
        
        kv.callback = [=](){ int c=(*clb)( args... );
            if( inp.is_closed () ){ return -1; } 
            if( inp.is_waiting() ){ return  0; }
        return c; };

        ptr_t<task_t> task( 0UL, task_t() );
        task->flag  = TASK_STATE::OPEN;
        task->addr  = append( kv ); 
        task->sign  = &obj;

        if( task->addr==nullptr ){ if( is_std( kv.fd ) ){  

            return loop_add( coroutine::add( COROUTINE(){
            coBegin; 

                while( (*clb)( args... )>=0 )
                     { coDelay( 100 ); }

            coFinish
            }));

        } else { return loop_add( cb, args... ); }}

    wake(); return task; }

    template< class... T >
    ptr_t<task_t> loop_add( const T&... args ) const noexcept {
    ptr_t<task_t> tsk = obj->ev_queue.add( args... ); wake(); return tsk; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & KV_STATE_SLEEP; }

    ulong get_delay() const noexcept { return get_delay_ms(); }

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
        while( obj->ev_queue.next() == 1 && batch() ){ return 1; } 
        set_timeout(obj->ev_queue.get_delay());
        auto stamp = process::now();

        /* TIMEOUT KILLER */
        do   { auto x=obj->kv_queue.first(); while( x != nullptr ){
               auto y=x->next; if( x->data.timeout==0 ) { break; }
        if   ( x->data.flag & TASK_STATE::USED ){ x=y; continue; }
        if   ( x->data.timeout < stamp ) /*--*/ { remove(x); }
        else { break; } x=y; }} while(0);

        /* CLOSED KILLER */
        if   ( obj->kv_queue.next() != nullptr ){ 
        if   ( obj->kv_queue.get ()->data.callback()==-1 )
             { remove( obj->kv_queue.get() );  }}

        /* IO DETECTION */
        obj->state |=  KV_STATE_SLEEP;
        obj->idx=kevent( obj->pd, NULL, 0, &obj->ev, obj->ev.size(), &get_delay_tm() );
        obj->state &=~ KV_STATE_SLEEP;

        /* EXCECUTION */
        while( obj->idx > 0 ){ obj->idx--; auto x = obj->ev[ obj->idx ];
            
            if( x.filter==EVFILT_USER ){ continue; }
            auto y = obj->kv_queue.as ( x.udata );
            if( !obj->kv_queue.is_valid( y ) ){ continue; }

            if( x.flags & ( EV_ERROR    | EV_EOF       ) &&
              ( x.filter& ( EVFILT_WRITE| EVFILT_READ ))==0
            ) { remove( y ); continue; }

            if( y->data.flag & FLAG::KV_STATE_USED ){ continue; }
                y->data.flag|= FLAG::KV_STATE_USED;

            obj->ev_queue.add( coroutine::add( COROUTINE(){
            coBegin

                do { switch( y->data.callback() ) {
                case -1: remove(y); /*----------------------*/ coEnd; break;
                case  0: y->data.flag &=~ FLAG::KV_STATE_USED; coEnd; break;
                case  1: /*------------------*/ break; } coNext; } while(1);

            coFinish
            }));

        }   clear_timeout(); obj->batch = NODEPP_MAX_BATCH_SIZE; return 1; }

};}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_POLL_NPOLL

namespace nodepp { class kernel_t {
private:

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
         KV_STATE_WRITE   = 0b00000001,
         KV_STATE_READ    = 0b00000010,
         KV_STATE_EDGE    = 0b10000000,
         KV_STATE_USED    = 0b00000100,
         KV_STATE_AWAIT   = 0b00001100,
         KV_STATE_SLEEP   = 0b01000000,
         KV_STATE_CLOSED  = 0b00001000
    };

    struct kevent_t { public:
        function_t<int> callback;
        ulong timeout; int fd, flag; 
    };

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

    void off( ptr_t<task_t> address ) const noexcept { clear( address ); }

    void clear( ptr_t<task_t> address ) const noexcept {
         if( address.null() ) /*--------------*/ { return; }
         if( address->flag & TASK_STATE::CLOSED ){ return; }
             address->flag = TASK_STATE::CLOSED;
    }

    /*─······································································─*/

    ulong size() const noexcept { return obj->ev_queue.size() + obj->probe.get() + obj.count()-1; }

    void clear() const noexcept { /*--*/ obj->ev_queue.clear(); obj->probe.clear(); }
    
    bool should_close() const noexcept { return empty() || NODEPP_SHTDWN(); }

    bool empty() const noexcept { return size()==0; }

    /*─······································································─*/

    bool is_sleeping() const noexcept { return obj->state & KV_STATE_SLEEP; }

    ulong get_delay() const noexcept { return get_delay_ms(); }

    int   wake() const noexcept { return -1; }

    /*─······································································─*/

    template< class T, class U, class... W >
    ptr_t<task_t> poll_add ( T str, int /*unused*/, U cb, ulong timeout=0, const W&... args ) const noexcept {

        auto time = type::bind( timeout>0 ? timeout + process::now() : timeout );
        auto clb  = type::bind( cb ); 
        
        return obj->ev_queue.add( coroutine::add( COROUTINE(){
        coBegin 

            if( *time > 0 && *time < process::now() ){ coEnd; }
            if( is_std( str.get_fd() ) ) /**/ { coDelay(100); }

            coSet(0); return (*clb)( args... ) >= 0 ? 1 : -1; 

        coFinish
        }));

    }

    template< class T, class... V >
    ptr_t<task_t> loop_add ( T cb, const V&... args ) const noexcept {
        return obj->ev_queue.add( cb, args... );
    }

    /*─······································································─*/

    inline int next() const {

        while( obj->ev_queue.next() == 1 ){ return 1; } 

        obj->state |=  KV_STATE_SLEEP;
        set_timeout(obj->ev_queue.get_delay());
        process::delay( get_delay_ms() );
        clear_timeout();
        obj->state &=~ KV_STATE_SLEEP;

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