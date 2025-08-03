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
protected:

    struct NODE {
        uchar /*--*/ ctx=0;
        mutex_t /**/ mtx  ;
    };  ptr_t<NODE>  obj  ;

public:

    semaphore_t() :obj( new NODE() ){}
    virtual ~semaphore_t() noexcept {}
    
    /*─······································································─*/

    void wait( uchar count ) const noexcept { goto check; 

        loop : worker::yield();
        check: obj->mtx.lock();

          if( obj->ctx >= obj.count() ){ obj->ctx =0; }
          if( obj.count()>0 ){ obj->ctx%=obj.count(); } 
          if( obj->ctx != count % obj.count() ) 
            { obj->mtx.unlock(); goto loop; }

        obj->mtx.unlock();

    }
    
    /*─······································································─*/

    void wait() const noexcept { goto check;

        loop : worker::yield();
        check: obj->mtx.lock(); 

          if( obj->ctx % 2 != 0 )
            { obj->mtx.unlock(); goto loop; }
            
        obj->mtx.unlock();

    }

    void release() const noexcept {
        obj->mtx.lock  (); ++obj->ctx;
        obj->mtx.unlock();
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif