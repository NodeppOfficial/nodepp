/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SEMAPHORE
#define NODEPP_SEMAPHORE

/*────────────────────────────────────────────────────────────────────────────*/

#include "mutex.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class semaphore_t {
public:

    semaphore_t() : obj( new NODE() ){}

   ~semaphore_t() noexcept {
        if( obj->addr == (void*)this )
          { release(); }
    };
    
    /*─······································································─*/

    void wait( uchar count ) const noexcept { 
        
        goto check; loop: worker::yield();

        check:
            obj->mutex.lock(); 
            if( obj->ctx >= obj.count() ) obj->ctx = 0;
            if( obj.count()>0 ) obj->ctx%=obj.count(); 
            if( obj->ctx != count%obj.count() ) 
              { obj->mutex.unlock(); goto loop; }
            obj->addr=(void*)this;
            obj->mutex.unlock();

    }
    
    /*─······································································─*/

    void wait() const noexcept { goto check;

        loop: worker::yield();
        
        check:
            obj->mutex.lock(); 
            if((obj->ctx%2) != 0 )
              { obj->mutex.unlock(); goto loop; }
          ++obj->ctx; obj->addr=(void*)this;
            obj->mutex.unlock();

    }

    void release() const noexcept {
        obj->mutex.lock();
        obj->addr=nullptr; 
      ++obj->ctx; 
        obj->mutex.unlock();
    }

protected:

    struct NODE {
        void*   addr=nullptr;
        uchar   ctx=0;
        mutex_t mutex;
    };  ptr_t<NODE> obj;

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif