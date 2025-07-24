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

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
    #include "windows/mutex.cpp"
#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #include "posix/mutex.cpp"
#else
    #error "This OS Does not support mutex.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif