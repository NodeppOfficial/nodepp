/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TASK
#define NODEPP_TASK

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { struct task_t/**/ { int flag=0x00; void *addr, *sign; }; }
namespace nodepp { struct TASK_STATE { enum TYPE {
    UNKNOWN = 0b00000000,
    OPEN    = 0b00000001,
    USED    = 0b00000010,
    CLOSED  = 0b00000100,
};};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { struct POLL_STATE { enum FLAG {
    UNKNOWN = 0b00000000,
    READ    = 0b00000010,
    WRITE   = 0b00000001,
    EDGE    = 0b10000000
};}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif
