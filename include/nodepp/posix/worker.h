/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_WORKER
#define NODEPP_POSIX_WORKER

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class worker_t { 
private:

    enum STATE {
         STATE_UNKNOWN = 0b00000000,
         STATE_OPEN    = 0b00000001,
         STATE_CLOSE   = 0b00000010,
         STATE_AWAIT   = 0b00000111,
    };

protected:

    struct NODE {
        atomic_t<char> state;
        atomic_t<int>  delay;
        function_t<int>   cb;
        pthread_t         id;
    };  ptr_t<NODE> obj;

    static void* callback( void* arg ){
        auto self = type::cast<worker_t>(arg); 
        self->obj->state.set( STATE::STATE_OPEN );

        while( !self->is_closed() && self->obj->cb()>=0 ){
        auto info = coroutine::getno();
        
        if( info.delay>0 ){ 
            self->obj->delay.set(type::cast<int>(info.delay));
            worker::delay( info.delay ); 
        } else { 
            self->obj->delay.set(1);
            worker::yield();
        }}

    self->obj->state.set( STATE::STATE_CLOSE );
    worker::exit(); return nullptr; }

public:

    template< class T, class... V >
    worker_t( T cb, const V&... arg ) noexcept : obj( new NODE() ){
        auto clb = type::bind(cb);
        obj->cb  = function_t<int>([=](){ return (*clb)(arg...); });
    }
    
    /*─······································································─*/

    /*----*/ worker_t() noexcept : obj( new NODE ) {}

    virtual ~worker_t() noexcept { if( obj.count()>1 ){ return; } free(); }
    
    /*─······································································─*/

    pthread_t pid() const noexcept { return obj->id; }
    void     free() const noexcept { obj->state.set(STATE::STATE_AWAIT); }
    void      off() const noexcept { obj->state.set(STATE::STATE_AWAIT); }
    void    close() const noexcept { obj->state.set(STATE::STATE_AWAIT); }
    
    /*─······································································─*/

    ulong get_delay() const noexcept {
          auto   data= obj->delay.get();
          return data==0 ? 1000 : data;
    }
    
    /*─······································································─*/

    bool is_closed() const noexcept { 
        char   x = obj->state.get();
        return x ==0x00 || ( x & STATE::STATE_CLOSE );
    }
    
    /*─······································································─*/

    int emit() const noexcept {
        if( obj->state.get() != 0x00 ){ return 0; } auto self = type::bind( this );
        
        auto pth = pthread_create( &obj->id, NULL, &callback, (void*) &self );
        if ( pth!= 0 ){ return -1; } pthread_detach( obj->id ); 
        
        process::add( coroutine::add( COROUTINE(){
        coBegin
            while( self->obj->state.get()==0x00 ) { coNext; }
            while( self->obj->state.get()&STATE::STATE_OPEN )
                 { coDelay( self->get_delay() ); }
        coFinish
        })); 
        
    return 1; }

    /*─······································································─*/

    int add() const noexcept { return emit(); }

    /*─······································································─*/

    int await() const noexcept {
        if( obj->state.get() != 0x00 ){ return 0; } auto self = type::bind( this );
        
        auto pth = pthread_create( &obj->id, NULL, &callback, (void*) &self );
        if ( pth!= 0 ){ return -1; } pthread_detach( obj->id ); 
        
        process::await( coroutine::add( COROUTINE(){
        coBegin
            while( self->obj->state.get()==0x00 ) { coNext; }
            while( self->obj->state.get()&STATE::STATE_OPEN )
                 { coDelay( self->get_delay() ); }
        coFinish
        }));
        
    return 1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/