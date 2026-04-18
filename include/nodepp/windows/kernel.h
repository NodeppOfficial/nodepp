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
#define NODEPP_POLL_IOCP
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifdef NODEPP_POLL_IOCP

/*────────────────────────────────────────────────────────────────────────────*/

#include <winsock2.h>
#include <mswsock.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class kernel_t {
private:

    using HPOLLFD = OVERLAPPED_ENTRY;

    enum FLAG { 
         KV_STATE_UNKNOWN = 0b00000000, 
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
        obj->kv_queue.erase( kv );
        
    return 0; }

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

    template< class T >
    void iocp_execute_callback( T* address ) const noexcept {

        if( address == nullptr ) { return; } auto y = address;
        if( y->data.flag & FLAG::KV_STATE_USED ){ return; }
            y->data.flag|= FLAG::KV_STATE_USED;

        obj->ev_queue.add( coroutine::add( COROUTINE(){
        coBegin

            do { switch( y->data.callback() ) {
            case -1: remove(y); /*---------------------*/ coEnd; break;
            case  0: y->data.flag&=~ FLAG::KV_STATE_USED; coEnd; break;
            case  1: /*-----------------*/ break; } coNext; } while(1);

        coFinish
        }));

    }

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
        if( obj.count()==1 ){ return -1; }}
        if( obj->kv_queue.empty() && tasks==0 ){ 
        if( obj.count()> 1 ){ return -1; }}
    return get_timeout(); }

protected:

    struct NODE {

        uchar batch = NODEPP_MAX_BATCH_SIZE;
        ulong /*--*/ timeout; int state;
        loop_t /*------*/ ev_queue;
        queue_t<kevent_t> kv_queue;
        probe_t /*-----*/ probe;
        ptr_t<HPOLLFD>    ev;
        HANDLE pd; ULONG idx;

       ~NODE(){ CloseHandle( pd ); }
    };  ptr_t<NODE> obj;

public:

    kernel_t() : obj( new NODE() ) {
        obj->pd = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
        if( obj->pd == NULL ){ throw except_t("Can't Initialize kernel_t"); }
        obj->ev.resize( NODEPP_MAX_BATCH_SIZE );
    }

   ~kernel_t() noexcept { 
        if( obj.count() > 1 || obj->state & KV_STATE_CLOSED )
          { return; } /*----*/ obj->state = KV_STATE_CLOSED;
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
    //  if( cb( args... )==-1 ){ return nullptr; }

        kevent_t       kv; 
        kv.flag      = flag;
        kv.timeout   = timeout==0 ? 0 : process::now() + timeout;
        kv.fd        = (HANDLE) inp.get_fd(); auto clb = type::bind( cb );
        
        kv.callback  = [=](){ 
            int c=(*clb)( args... );
            if( inp.is_closed () ){ return -1; } 
            if( inp.is_waiting() ){ return  0; }
        return c; };

        ptr_t<task_t> task( 0UL, task_t() ); 
        task->flag   = TASK_STATE::OPEN;
        task->addr   = append( kv ); 
        task->sign   = &obj;

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
        obj->state |= KV_STATE_SLEEP;
        if( !GetQueuedCompletionStatusEx( 
            obj->pd , obj->ev .data(), obj->ev.size(), 
           &obj->idx, get_delay_ms (), FALSE )
        ) { obj->state &=~ KV_STATE_SLEEP; return 1; }
        obj->state &=~ KV_STATE_SLEEP; 
          
        /* EXCECUTION */
        while( obj->idx > 0 ){ obj->idx--; auto x = obj->ev[ obj->idx ];

            if( x.lpCompletionKey==(ULONG_PTR)NULL ){ continue; }

            auto y = obj->kv_queue.as  ( (void*) x.lpCompletionKey );
            if( !obj->kv_queue.is_valid( y ) ) /**/ { continue; }
            iocp_execute_callback( y );

        } 
        
    clear_timeout(); obj->batch = NODEPP_MAX_BATCH_SIZE; return 1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/