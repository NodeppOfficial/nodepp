/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_MUTEX
#define NODEPP_POSIX_MUTEX

/*────────────────────────────────────────────────────────────────────────────*/

#include <pthread.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace worker {

    inline void    delay( ulong time ){ process::delay(time); }
    inline void    yield(){ delay(TIMEOUT); sched_yield(); }
    inline pthread_t pid(){ return pthread_self(); }
    inline void     exit(){ pthread_exit(NULL); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class mutex_t {
protected:

    struct NODE {
        atomic_t<bool> alive=1;
        pthread_mutex_t fd;
    };  ptr_t<NODE> obj;

public:

    mutex_t() : obj( new NODE() ) {
        if( pthread_mutex_init(&obj->fd,NULL)!=0 )
          { throw except_t("Cant Start Mutex");  }
            /*-----------------*/ obj->alive=1;
    }

    virtual ~mutex_t() noexcept {
        if( obj->alive == 0 ){ return; }
        if( obj.count() > 1 ){ return; } 
    free(); }
    
    /*─······································································─*/

    void free() const noexcept {
        if( obj->alive == 0 ){ return; }
        /*----------*/ obj->alive = 0;
        pthread_mutex_destroy(&obj->fd);
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
        int c=callback( args... ); unlock(); return 1;
    } 
    
    /*─······································································─*/

    template< class T, class... V >
    inline void lock( T callback, const V&... args ) const noexcept {
        if( obj->alive == 0 ){ return; }
        lock(); callback( args... ); unlock(); 
    }

    template< class T, class... V >
    inline int _lock( T callback, const V&... args ) const noexcept {
        if( obj->alive == 0 ){ return -1; }
        if( !_lock() ) /*-*/ { return -2; }
        callback( args... ); unlock(); return 1;
    } 
    
    /*─······································································─*/

    void unlock() const noexcept { while( !_unlock() ){ worker::yield(); } }
    void lock()   const noexcept { while( !_lock  () ){ worker::yield(); } }
    
    /*─······································································─*/

    inline bool _unlock() const noexcept { return pthread_mutex_unlock(&obj->fd)==0; }
    inline bool _lock()   const noexcept { return pthread_mutex_lock  (&obj->fd)==0; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/