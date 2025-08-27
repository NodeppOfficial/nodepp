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
#include "mutex.cpp"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class worker_t { 
private:

    struct waiter { bool blk; bool out; }; 

protected:

    struct NODE {
        function_t<int> cb;
        HANDLE /**/ thread;
        bool *out, state=0;
        DWORD /*-----*/ id; 
        mutex_t /*--*/ mtx;
    };  ptr_t<NODE> obj;

    static DWORD WINAPI callback( LPVOID arg ){
        auto self = type::cast<worker_t>(arg);
        self->obj->mtx.emit([=](){ self->obj->state=1; });
        while( self->obj->cb.emit()>=0 ){ worker::yield(); } 
        self->obj->mtx.emit([=](){ self->free(); });
        /*-------*/ delete self; worker::exit(); 
    return 0; }

public:

    template< class T, class... V >
    worker_t( T cb, const V&... arg ) noexcept : obj( new NODE() ){

        ptr_t<waiter> wrk = new waiter();
        auto clb = type::bind(cb);
        wrk->blk = 0; wrk->out =1;
        obj->out = &wrk->out;

        obj->cb = function_t<int>([=](){ 
            if( wrk->out==0 ){ return -1; }
            if( wrk->blk==1 ){ return  1; } 
                wrk->blk =1; int rs=(*clb)( arg... );
            if( clb.null()  ){ return -1; }  
                wrk->blk =0;   return !wrk->out?-1:rs;
        }); 

    }
    
    /*─······································································─*/

    worker_t() noexcept : obj( new NODE ) {}

    virtual ~worker_t() noexcept { if( obj.count() > 1 ){ return; } free(); }
    
    /*─······································································─*/

    void free() const noexcept {
        if( obj->state == 0 ){ return; } --_TASK_; obj->state=0;
        if( obj->thread!= nullptr ){ CloseHandle( obj->thread ); }
    }
    
    /*─······································································─*/

    int    pid() const noexcept { return type::cast<int>(obj->id); }
    void   off() const noexcept { process::clear( obj->out ); }
    void close() const noexcept { process::clear( obj->out ); }
    
    /*─······································································─*/

    int add() const noexcept {
        if( obj->state == 1 ){ return 0; } auto self = new worker_t( *this );

        obj->thread = CreateThread( NULL, 0, &callback, (void*)self, 0, &obj->id );
        if( obj->thread == NULL ){ delete self; return -1; }
        
        WaitForSingleObject( obj->thread, 0 ); ++_TASK_;

        while( obj->state==0 ){ /*------------*/ } 
    //  while( obj->state==1 ){ process::next(); } 

        return 1;
    }

    /*─······································································─*/

    int await() const noexcept {
        if( obj->state == 1 ){ return 0; } auto self = new worker_t( *this );

        obj->thread = CreateThread( NULL, 0, &callback, (void*)self, 0, &obj->id );
        if( obj->thread == NULL ){ delete self; return -1; }

        WaitForSingleObject( obj->thread, 0 ); ++_TASK_;

        while( obj->state==0 ){ /*------------*/ } 
        while( obj->state==1 ){ process::next(); } 

        return 1;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/