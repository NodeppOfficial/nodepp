/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_WORKER
#define NODEPP_WINDOWS_WORKER

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
        HANDLE thread; DWORD id;
        ptr_t<kernel_t>  krn;
        atomic_t<char> state;
        function_t<int>   cb;
    };  ptr_t<NODE> obj;

    static DWORD WINAPI callback( LPVOID arg ){
        auto self = type::cast<worker_t>(arg);
        self->obj->state.set( STATE::WK_STATE_OPEN );

        while( !self->is_closed() && self->obj->cb()>=0 ){
        auto info = coroutine::getno();
        
        if( info.delay>0 ){ 
            worker::delay( info.delay ); 
        } else { 
            worker::yield();
        }}

        self->obj->state.set( STATE::WK_STATE_CLOSE ); 
        self->obj->krn->emit(); 
        self->obj->krn.reset();

    delete self; worker::exit(); return 0; }

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

    DWORD   pid() const noexcept { return obj->id; }
    void   free() const noexcept { obj->state.set(STATE::WK_STATE_AWAIT); }
    void    off() const noexcept { obj->state.set(STATE::WK_STATE_AWAIT); }
    void  close() const noexcept { obj->state.set(STATE::WK_STATE_AWAIT); }
    
    /*─······································································─*/

    bool is_closed() const noexcept { 
        char   x = obj->state.get();
        return x ==0x00 || ( x & STATE::WK_STATE_CLOSE );
    }
    
    /*─······································································─*/

    int emit() const noexcept {
        if( obj->state.get() != 0x00 ){ return 0; }
        
        obj->krn = type::bind( process::NODEPP_EV_LOOP() );

        /**/obj->thread =  CreateThread( NULL,0, &callback, (void*) new worker_t(*this), 0, &obj->id );
        if( obj->thread == NULL ){ return -1; } WaitForSingleObject( obj->thread,0 );  

        while( obj->state.get()==0x00 ) { /*unused*/ }
        
    return 1; }

    /*─······································································─*/

    int add() const noexcept { return emit(); }

    /*─······································································─*/

    int await() const noexcept {
        if( obj->state.get() != 0x00 ){ return 0; }
        
        obj->krn = type::bind( process::NODEPP_EV_LOOP() );

        /**/obj->thread =  CreateThread( NULL,0, &callback, (void*) new worker_t(*this), 0, &obj->id );
        if( obj->thread == NULL ){ return -1; } WaitForSingleObject( obj->thread,0 );  

        while( obj->state.get()==0x00 ) { /*unused*/ }
        while( obj->state.get()&STATE::WK_STATE_OPEN )
             { process::next(); }
        
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
        HANDLE thread; DWORD id;
        atomic_t<char> state;
        function_t<int>   cb;
    };  ptr_t<NODE> obj;

    static DWORD WINAPI callback( LPVOID arg ){
        auto self = type::cast<worker_t>(arg);
        self->obj->state.set( STATE::WK_STATE_OPEN );

        while( !self->is_closed() && self->obj->cb()>=0 ){
        auto info = coroutine::getno();
        
        if( info.delay>0 ){ 
            worker::delay( info.delay ); 
        } else { 
            worker::yield();
        }}

    self->obj->state.set( STATE::WK_STATE_CLOSE ); 
    worker::exit(); return 0; }

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

    DWORD   pid() const noexcept { return obj->id; }
    void   free() const noexcept { obj->state.set(STATE::WK_STATE_AWAIT); }
    void    off() const noexcept { obj->state.set(STATE::WK_STATE_AWAIT); }
    void  close() const noexcept { obj->state.set(STATE::WK_STATE_AWAIT); }
    
    /*─······································································─*/

    bool is_closed() const noexcept { 
        char   x = obj->state.get();
        return x ==0x00 || ( x & STATE::WK_STATE_CLOSE );
    }
    
    /*─······································································─*/

    int emit() const noexcept {
        if( obj->state.get() != 0x00 ){ return 0; } auto self = type::bind( this );

        /**/obj->thread =  CreateThread( NULL,0, &callback, (void*) &self, 0, &obj->id );
        if( obj->thread == NULL ){ return -1; } WaitForSingleObject( obj->thread,0 );  
        
        process::add( coroutine::add( COROUTINE(){
        coBegin
            while( self->obj->state.get()==0x00 )/**/{ coNext; }
            while( self->obj->state.get()&STATE::WK_STATE_OPEN )
                 { coDelay( 1000 ); }
        coFinish
        })); 
        
    return 1; }

    /*─······································································─*/

    int add() const noexcept { return emit(); }

    /*─······································································─*/

    int await() const noexcept {
        if( obj->state.get() != 0x00 ){ return 0; } auto self = type::bind( this );

        /**/obj->thread =  CreateThread( NULL,0, &callback, (void*) &self, 0, &obj->id );
        if( obj->thread == NULL ){ return -1; } WaitForSingleObject( obj->thread,0 );  
        
        process::await( coroutine::add( COROUTINE(){
        coBegin
            while( self->obj->state.get()==0x00 )/**/{ coNext; }
            while( self->obj->state.get()&STATE::WK_STATE_OPEN )
                 { coDelay( 1000 ); }
        coFinish
        })); 
        
    return 1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/