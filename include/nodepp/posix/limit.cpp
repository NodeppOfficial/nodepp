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
#include <sys/resource.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit { ulong _count_=0;

    uint get_hard_fileno() { struct rlimit limit;
        if( getrlimit( RLIMIT_NOFILE, &limit )==0 ) 
          { return limit.rlim_max; } return 1024;
    }

    uint get_soft_fileno() { struct rlimit limit;
        if( getrlimit( RLIMIT_NOFILE, &limit )==0 ) 
          { return limit.rlim_cur; } return 1024;
    }

    int set_hard_fileno( uint value ) {
        struct rlimit limit;
        limit.rlim_max = value;
        limit.rlim_cur = get_soft_fileno();
        return setrlimit( RLIMIT_NOFILE, &limit );
    }

    int set_soft_fileno( uint value ) {
        struct rlimit limit;
        limit.rlim_cur = value;
        limit.rlim_max = get_hard_fileno();
        return setrlimit( RLIMIT_NOFILE, &limit );
    }
    
    /*─······································································─*/
    
    ulong fileno_count(){ return _count_; }

    bool  fileno_ready(){ return _count_ < get_soft_fileno(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit { class probe_t{ public:
    virtual ~probe_t() noexcept { --_count_; }
    /*----*/ probe_t() noexcept { ++_count_; }
};}}

/*────────────────────────────────────────────────────────────────────────────*/
