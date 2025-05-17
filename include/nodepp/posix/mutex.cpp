/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <pthread.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace worker {

    void delay( ulong time ){ process::delay(time); }

    int    pid(){ return (int)pthread_self(); }

    void  exit(){ pthread_exit(NULL); }

    void yield(){ 
        delay( TIMEOUT );
        sched_yield();
    }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class mutex_t {
protected:

    struct NODE {
        void* addr = nullptr;
        bool  state= 0;
        pthread_mutex_t fd;
    };  ptr_t<NODE> mutex;

public:

    mutex_t() : mutex( new NODE() ) {
        if( pthread_mutex_init(&mutex->fd,NULL) != 0 )
          { process::error("Cant Start Mutex"); }
            mutex->addr=nullptr; mutex->state=1;
    }

    virtual ~mutex_t() noexcept {
        if( mutex->state== 0 )          { return;   }
        if( mutex->addr == (void*)this ){ unlock(); }
        if( mutex.count() > 1 )         { return;   } free();
    }
    
    /*─······································································─*/

    void free() const noexcept {
        if( mutex->state == 0 ){ return; }
            mutex->state =  0;
        pthread_mutex_destroy(&mutex->fd);
    }
    
    /*─······································································─*/

    void unlock() const noexcept { 
        while( pthread_mutex_unlock(&mutex->fd)!=0 )
             { worker::yield(); } 
        mutex->addr = nullptr;
    }

    void lock() const noexcept { 
        while( pthread_mutex_lock(&mutex->fd)!=0 )
             { worker::yield(); } 
        mutex->addr = (void*)this;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/