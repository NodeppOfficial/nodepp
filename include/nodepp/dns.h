/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_DNS
#define NODEPP_DNS

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
    #include "url.h"
    #include "socket.h"
    #include "windows/dns.cpp"
#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #include "url.h"
    #include "socket.h"
    #include "posix/dns.cpp"
#else
    #error "This OS Does not support dns.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif