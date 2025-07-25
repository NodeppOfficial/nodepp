/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SOCKET
#define NODEPP_SOCKET

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
    #include "os.h"
    #include "limit.h"
    #include "stream.h"
    #include "windows/socket.cpp"
#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #include "os.h"
    #include "limit.h"
    #include "stream.h"
    #include "posix/socket.cpp"
#else
    #error "This OS Does not support socket.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif