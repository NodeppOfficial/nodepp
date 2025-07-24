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
#include <windows.h>
#include <processthreadsapi.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace worker {

    void delay( ulong time ){ process::delay(time); }
    void yield(){ delay(TIMEOUT); SwitchToThread(); }
    int    pid(){ return GetCurrentThreadId(); }
    void  exit(){ ExitThread(0); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class mutex_t {
protected:

    struct NODE {
        void*  addr = nullptr;
        bool   state= 0;
        HANDLE fd;
    };  ptr_t<NODE> mutex;

public:

    mutex_t() : mutex( new NODE() ) {
        mutex->fd   = CreateMutex( NULL, 0, NULL );
        mutex->addr = nullptr;
        mutex->state= 1;
        if( mutex->fd == NULL )
          { throw except_t("Cant Start Mutex"); }
    }

   ~mutex_t() noexcept {
        if( mutex->state== 0 )          { return;   }
        if( mutex->addr == (void*)this ){ unlock(); }
        if( mutex.count() > 1 )         { return;   } free();
    }
    
    /*─······································································─*/

    void free() const noexcept {
         if( mutex->state == 0 ){ return; }
             mutex->state =  0; CloseHandle( mutex->fd );
    }
    
    /*─······································································─*/

    template< class T, class... V >
    void emit( T callback, const V&... args ) const noexcept {
         lock(); callback( args... ); unlock();
    }
    
    /*─······································································─*/

    void unlock() const noexcept { 
        while( ReleaseMutex( mutex->fd )==0 )
             { worker::yield(); }
        mutex->addr = nullptr;
    }

    void lock() const noexcept { 
        while( WaitForSingleObject( mutex->fd, 0 ) != 0 )
             { worker::yield(); }
        mutex->addr = (void*)this;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/