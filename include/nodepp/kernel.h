/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP__KERNEL
#define NODEPP__KERNEL

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT_SCHEDULER

#if  ( NODEPP_OS == NODEPP_OS_WINDOWS )
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_IOCP
#elif( NODEPP_OS == NODEPP_OS_LINUX )
    #include <linux/version.h>
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
        #define  NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_IOURING
        #include "posix/uring.h"
    #else 
        #define  NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_EPOLL
    #endif
#elif( NODEPP_OS == NODEPP_OS_FRBSD ) || ( NODEPP_OS == NODEPP_OS_APPLE )
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_KQUEUE
#else
    #define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_LITE
#endif

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if   NODEPP_KERNEL == NODEPP_KERNEL_WINDOWS
    #include "invoker.h"
    #include "windows/kernel.h"
#elif NODEPP_KERNEL == NODEPP_KERNEL_POSIX
    #include "invoker.h"
    #include "posix/kernel.h"
#else
    #error "This OS Does not support kernel.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/