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
        HANDLE fd; bool state=0;
    };  ptr_t<NODE> obj;

public:

    mutex_t() : obj( new NODE() ) {
        obj->fd   = CreateMutex( NULL, 0, NULL );
        if( obj->fd == NULL )
          { throw except_t("Cant Start Mutex"); }
            /*-----------------*/ obj->state=1; 
    }

    virtual ~mutex_t() noexcept {
        if( obj->state == 0 ){ return; }
        if( obj.count() > 1 ){ return; } 
    free(); }
    
    /*─······································································─*/

    void free() const noexcept {
         if( obj->state == 0 ){ return; }
             obj->state =  0; CloseHandle( obj->fd );
    }
    
    /*─······································································─*/

    template< class T, class... V >
    void emit( T callback, const V&... args ) const noexcept {
         lock(); callback( args... ); unlock();
    }
    
    /*─······································································─*/

    void unlock() const noexcept { while( !_unlock() ){ worker::yield(); } }
    void lock()   const noexcept { while( !_lock  () ){ worker::yield(); } }
    
    /*─······································································─*/

    inline bool _unlock() const noexcept { return ReleaseMutex( obj->fd )!=0; }
    inline bool _lock()   const noexcept { return WaitForSingleObject( obj->fd,0 )==0; }

};}

/*────────────────────────────────────────────────────────────────────────────*/
