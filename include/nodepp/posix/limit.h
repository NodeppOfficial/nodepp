/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POSIX_LIMIT
#define NODEPP_POSIX_LIMIT

/*────────────────────────────────────────────────────────────────────────────*/

#include <sys/resource.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit { ulong _count_=0;

    inline uint get_hard_fileno() { struct rlimit limit;
        if( getrlimit( RLIMIT_NOFILE, &limit )==0 ) 
          { return limit.rlim_max; } return 1024;
    }

    inline uint get_soft_fileno() { struct rlimit limit;
        if( getrlimit( RLIMIT_NOFILE, &limit )==0 ) 
          { return limit.rlim_cur; } return 1024;
    }

    inline int set_hard_fileno( uint value ) {
        struct rlimit limit;
        limit.rlim_max = value;
        limit.rlim_cur = get_soft_fileno();
        return setrlimit( RLIMIT_NOFILE, &limit );
    }

    inline int set_soft_fileno( uint value ) {
        struct rlimit limit;
        limit.rlim_cur = value;
        limit.rlim_max = get_hard_fileno();
        return setrlimit( RLIMIT_NOFILE, &limit );
    }
    
    /*─······································································─*/
    
    inline ulong fileno_count(){ return _count_; }

    inline bool  fileno_ready(){ return _count_ < get_soft_fileno(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/