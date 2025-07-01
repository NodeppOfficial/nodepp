/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef  NODEPP_POLL
#define  NODEPP_POLL
#include "wait.h"

#if   _POLL == NODEPP_POLL_WPOLL
    #include "windows/poll.cpp"
#elif _POLL == NODEPP_POLL_EPOLL
    #include "posix/epoll.cpp"
#elif _POLL == NODEPP_POLL_KPOLL
    #include "posix/kpoll.cpp"
#elif _POLL == NODEPP_POLL_POLL
    #include "posix/poll.cpp"
#else
    #include "posix/npoll.cpp"
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/