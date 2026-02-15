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

#ifndef NODEPP_POLL_NPOLL

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class worker_t { 
private:

    enum STATE {
         WK_STATE_UNKNOWN = 0b00000000,
         WK_STATE_OPEN    = 0b00000001,
         WK_STATE_CLOSE   = 0b00000010,
         WK_STATE_AWAIT   = 0b00000111,
    };

protected:

    struct NODE {
        pthread_t id; 
        atomic_t<char> state;
        ptr_t<kernel_t>  krn;
        function_t<int>   cb;
    };  ptr_t<NODE> obj;

    static void* callback( void* arg ){
        auto self = type::cast<worker_t>(arg); 
        self->obj->state=STATE::WK_STATE_OPEN;

        while( !self->is_closed() && self->obj->cb()>=0 ){
        auto info = coroutine::getno();

        if( info.delay>0 ){ 
            worker::delay( info.delay ); 
        } else { 
            worker::yield();
        }}

    self->obj->state = STATE::WK_STATE_CLOSE; 
    self->obj->krn->emit(); /**/ delete self; 
    worker::exit(); return nullptr; }

public:

    template< class T, class... V >
    worker_t( T cb, const V&... arg ) noexcept : obj( new NODE() ){
        auto clb = type::bind(cb);
        obj->cb  = function_t<int>([=](){ return (*clb)(arg...); });
    }
    
    /*─······································································─*/

    worker_t() noexcept : obj( new NODE ) {}

   ~worker_t() noexcept { if( obj.count()>1 ){ return; } free(); }
    
    /*─······································································─*/

    void   free() const noexcept { obj->state = STATE::WK_STATE_AWAIT; }
    void    off() const noexcept { obj->state = STATE::WK_STATE_AWAIT; }
    void  close() const noexcept { obj->state = STATE::WK_STATE_AWAIT; }
    
    /*─······································································─*/

    bool is_closed() const noexcept { 
    char x = obj->state.get();
        return ( x & STATE::WK_STATE_CLOSE ) || x==0x00 ;
    }
    
    /*─······································································─*/

    int emit() const noexcept {
        if( obj->state != 0x00 ){ return 0; }
        
        obj->krn = type::bind( process::NODEPP_EV_LOOP() );

        auto pth = pthread_create( &obj->id, NULL, &callback, (void*) new worker_t(*this) );
        if ( pth!= 0 ){ return -1; } pthread_detach( obj->id ); 

        while( obj->state == 0x00 ) { /*unused*/ }
        
    return 1; }

    /*─······································································─*/

    int add() const noexcept { return emit(); }

    /*─······································································─*/

    int await() const noexcept { if( obj->state != 0x00 ){ return 0; } 
        
        obj->krn = type::bind( process::NODEPP_EV_LOOP() );

        auto pth = pthread_create( &obj->id, NULL, &callback, (void*) new worker_t(*this) );
        if ( pth!= 0 ){ return -1; } pthread_detach( obj->id ); 

        while( obj->state.get() ==0x00 ) /*----------*/ { /*-- unused --*/ }
        while( obj->state.get() & STATE::WK_STATE_OPEN ){ process::next(); }
        
    return 1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#else

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class worker_t { 
private:

    enum STATE {
         WK_STATE_UNKNOWN = 0b00000000,
         WK_STATE_OPEN    = 0b00000001,
         WK_STATE_CLOSE   = 0b00000010,
         WK_STATE_AWAIT   = 0b00000111,
    };

protected:

    struct NODE {
        pthread_t id; 
        atomic_t<char> state;
        ptr_t<kernel_t>  krn;
        function_t<int>   cb;
    };  ptr_t<NODE> obj;

    static void* callback( void* arg ){
        auto self = type::cast<worker_t>(arg); 
        self->obj->state=STATE::WK_STATE_OPEN;

        while( !self->is_closed() && self->obj->cb()>=0 ){
        auto info = coroutine::getno();
        
        if( info.delay>0 ){ 
            worker::delay( info.delay ); 
        } else { 
            worker::yield();
        }}

    self->obj->state = STATE::WK_STATE_CLOSE;
    worker::exit(); return nullptr; }

public:

    template< class T, class... V >
    worker_t( T cb, const V&... arg ) noexcept : obj( new NODE() ){
        auto clb = type::bind(cb);
        obj->cb  = function_t<int>([=](){ return (*clb)(arg...); });
    }
    
    /*─······································································─*/

    worker_t() noexcept : obj( new NODE ) {}

   ~worker_t() noexcept { if( obj.count()>1 ){ return; } free(); }
    
    /*─······································································─*/

    void   free() const noexcept { obj->state = STATE::WK_STATE_AWAIT; }
    void    off() const noexcept { obj->state = STATE::WK_STATE_AWAIT; }
    void  close() const noexcept { obj->state = STATE::WK_STATE_AWAIT; }
    
    /*─······································································─*/

    bool is_closed() const noexcept { 
    char x = obj->state.get();
         return ( x & STATE::WK_STATE_CLOSE ) || x==0x00 ;
    }
    
    /*─······································································─*/

    int emit() const noexcept { 
        
        if( obj->state != 0x00 ) { return 0; } auto self = type::bind( this );
        auto pth = pthread_create( &obj->id, NULL, &callback, (void*) &self );
        if ( pth!= 0 ){ return -1; } pthread_detach( obj->id ); 
        
        process::add( coroutine::add( COROUTINE(){
        coBegin
            while( self->obj->state.get() ==0x00 )/*-*/{ coNext; }
            while( self->obj->state.get() & STATE::WK_STATE_OPEN )
                 { coDelay( 1000 ); }
        coFinish
        })); 
        
    return 1; }

    /*─······································································─*/

    int add() const noexcept { return emit(); }

    /*─······································································─*/

    int await() const noexcept {

        if( obj->state != 0x00 ) { return 0; } auto self = type::bind( this );
        auto pth = pthread_create( &obj->id, NULL, &callback, (void*) &self );
        if ( pth!= 0 ){ return -1; } pthread_detach( obj->id ); 
        
        process::await( coroutine::add( COROUTINE(){
        coBegin
            while( self->obj->state.get() ==0x00 )/*-*/{ coNext; }
            while( self->obj->state.get() & STATE::WK_STATE_OPEN )
                 { coDelay( 1000 ); }
        coFinish
        }));
        
    return 1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/