/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SERIAL
#define NODEPP_SERIAL

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL == NODEPP_KERNEL_WINDOWS
    #include "fs.h"
    #include "windows/serial.cpp"
#elif _KERNEL == NODEPP_KERNEL_POSIX
    #include "fs.h"
    #include "posix/serial.cpp"
#else
    #error "This OS Does not support serial.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif