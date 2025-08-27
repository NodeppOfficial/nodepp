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

namespace nodepp { namespace limit { ulong _count_=0;
    
    int set_hard_fileno( uint value ) { return _setmaxstdio( value ); }

    int set_soft_fileno( uint value ) { return _setmaxstdio( value ); }
    
    /*─······································································─*/

    uint get_hard_fileno() { return _getmaxstdio(); }

    uint get_soft_fileno() { return _getmaxstdio(); }
    
    /*─······································································─*/
    
    ulong fileno_count(){ return _count_; }

    bool  fileno_ready(){ return _count_ < get_soft_fileno(); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit { class probe_t{ public:
    /*----*/ probe_t() noexcept { ++_count_; }
    virtual ~probe_t() noexcept { --_count_; }
};}}

/*────────────────────────────────────────────────────────────────────────────*/
