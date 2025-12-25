/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_MUTEX
#define NODEPP_WINDOWS_MUTEX

/*────────────────────────────────────────────────────────────────────────────*/

#include <windows.h>
#include <processthreadsapi.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace worker {

    inline void delay( ulong time ){ process::delay(time); }
    
    inline void yield(){ delay(TIMEOUT); SwitchToThread(); }

    inline DWORD  pid(){ return GetCurrentThreadId(); }

    inline void  exit(){ ExitThread(0); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class mutex_t {
protected:

    struct NODE {
        HANDLE /*---*/ fd;
        bool alive = true;
    };  ptr_t<NODE> obj;

public:

    mutex_t() : obj( new NODE() ) {
        obj->fd   = CreateMutex( NULL, 0, NULL );
        if( obj->fd == NULL )
          { throw except_t("Cant Start Mutex"); }
            /*-----------------*/ obj->alive=1; 
    }

    virtual ~mutex_t() noexcept {
        if( obj->alive == 0 ){ return; }
        if( obj.count() > 1 ){ return; } 
    free(); }
    
    /*─······································································─*/

    void free() const noexcept {
         if( obj->alive == 0 ){ return; }
             obj->alive =  0; CloseHandle( obj->fd );
    }
    
    /*─······································································─*/

    template< class T, class... V >
    int operator() ( T callback, const V&... args ) const noexcept { 
        return emit( callback, args... ); 
    }
    
    /*─······································································─*/

    template< class T, class... V >
    inline int emit( T callback, const V&... args ) const noexcept {
        if( obj->alive == 0 ){ return -1; }
        lock  (); int c=callback( args... ); 
        unlock(); /*------------*/ return c;
    }

    template< class T, class... V >
    inline int _emit( T callback, const V&... args ) const noexcept {
        if( obj->alive == 0 ){ return -1; }
        if( !_lock() ) /*-*/ { return -2; }
        int c=callback( args... ); unlock(); return c;
    } 
    
    /*─······································································─*/

    template< class T, class... V >
    inline void lock( T callback, const V&... args ) const noexcept {
        if( obj->alive == 0 )/*-*/{ return; }
        lock(); callback( args... ); unlock(); 
    }

    template< class T, class... V >
    inline int _lock( T callback, const V&... args ) const noexcept {
        if( obj->alive == 0 ){ return -1; }
        if( !_lock() ) /*-*/ { return -2; }
        callback( args... ); unlock(); return 1;
    } 
    
    /*─······································································─*/

    inline bool _unlock() const noexcept { return ReleaseMutex( obj->fd )!=0; }
    inline bool _lock()   const noexcept { return WaitForSingleObject( obj->fd,0 )==0; }
    
    /*─······································································─*/

    inline bool unlock() const noexcept { return ReleaseMutex( obj->fd )!=0; }
    inline bool lock()   const noexcept { return WaitForSingleObject( obj->fd,INFINITE )==0; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/