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

namespace nodepp { namespace { 
    
    mutex_t mtx;

    DWORD WINAPI sfunc( LPVOID arg ){
        auto cb = (function_t<int>*) arg;
        while((*cb)() >= 0 ){ worker::yield(); }
        mtx.lock(); process::threads--; mtx.unlock();
        delete cb; worker::exit(); return 0; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class worker_t { 
protected:

    struct NODE {
        DWORD            id;
        function_t<int>* cb;
        ptr_t<bool>     out;
        HANDLE       thread;
        bool        state=0;
    };  ptr_t<NODE> obj;

public: worker_t() noexcept : obj( new NODE ) {}

    virtual ~worker_t() noexcept {
        if( obj.count() > 1 ){ return; } 
        if( obj->state == 0 ){ return; } 
        CloseHandle( obj->thread );
    }
    
    /*─······································································─*/

    template< class T, class... V >
    worker_t( T cb, V&... arg ) noexcept : obj( new NODE() ){
        if( MAX_WORKERS!=0 && process::threads>=MAX_WORKERS ){ return; }
        ptr_t<T>    clb = new T( cb );
        ptr_t<bool> blk = new bool(0);
        ptr_t<bool> out = new bool(1);
        obj->cb = new function_t<int>([=](){ 
            if( *out==0 ){ return -1; }
            if( *blk==1 ){ return  1; } *blk = 1;
            int rs = ( *clb )(arg...);  *blk = 0;
            return *out!=1 ? -1 : rs; 
        }); obj->out = out;
    }
    
    /*─······································································─*/

    int    pid() const noexcept { return (int)obj->id; }

    void close() const noexcept { *obj->out = 0; }
    
    /*─······································································─*/

    int run() const noexcept { if( obj->state == 1 ){ return 0; } int c = 0; obj->state = 1;
        obj->thread = CreateThread( NULL, 0, &sfunc, (void*)obj->cb, 0, &obj->id );
        if( obj->thread != NULL ){ process::threads++; }
        WaitForSingleObject( obj->thread, 0 ); return obj->thread == NULL ? -1 : 0;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/