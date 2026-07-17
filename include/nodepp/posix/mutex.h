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

namespace nodepp { class mutex_t {
protected:

    struct NODE {
       ~NODE(){ pthread_mutex_destroy(&fd); }
        pthread_mutex_t fd;
    };  atomic_ptr_t<NODE> obj;

public:

    mutex_t() : obj( new NODE() ) {
        if( pthread_mutex_init(&obj->fd,NULL)!=0 )
          { NODEPP_THROW_ERROR("Cant Start Mutex");  }
    }
    
    /*─······································································─*/

    template< class T, class... V >
    int operator() ( T callback, const V&... args ) const noexcept { 
        return emit( callback, args... ); 
    }
    
    /*─······································································─*/

    template< class T, class... V >
    int emit( T callback, const V&... args ) const noexcept {
        lock  (); int c=callback( args... ); 
        unlock(); /*------------*/ return c;
    }

    template< class T, class... V >
    void lock( T callback, const V&... args ) const noexcept {
         lock(); callback( args... ); unlock(); 
    }
    
    /*─······································································─*/

    void unlock() const noexcept { while( !_unlock() ){ /*unused*/ } }
    void lock  () const noexcept { while( !_lock  () ){ /*unused*/ } }

    /*─······································································─*/

    bool _unlock() const noexcept { return pthread_mutex_unlock(&obj->fd)==0; }
    bool _lock  () const noexcept { return pthread_mutex_lock  (&obj->fd)==0; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/