/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MUTEX
#define NODEPP_MUTEX

/*────────────────────────────────────────────────────────────────────────────*/

#if !defined( NODEPP_THREAD_SUPPORTED )
    #error "This OS Does not support mutex.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
    #include "atomic.h"
    #include "windows/mutex.h"
#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #include "atomic.h"
    #include "posix/mutex.h"
#else
    #error "This OS Does not support mutex.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace mutex {

    template< class T, class... V >
    function_t<int,V...> add( mutex_t mut, T cb, const V&... args ){
        return [=](){ return mut.emit( cb, args... ); };
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif