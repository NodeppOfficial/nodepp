/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit {

    uint get_hard_fileno() { return _getmaxstdio(); }

    uint get_soft_fileno() { return _getmaxstdio(); }

    int set_hard_fileno( uint value ) {
        return _setmaxstdio( value );
    }

    int set_soft_fileno( uint value ) {
        return _setmaxstdio( value );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/
