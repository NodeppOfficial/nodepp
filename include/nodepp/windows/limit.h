/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WINDOWS_LIMIT
#define NODEPP_WINDOWS_LIMIT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace limit { ulong _count_=0;
    
    inline int set_hard_fileno( uint value ) { return _setmaxstdio( value ); }

    inline int set_soft_fileno( uint value ) { return _setmaxstdio( value ); }
    
    /*─······································································─*/

    inline uint get_hard_fileno() { return _getmaxstdio(); }

    inline uint get_soft_fileno() { return _getmaxstdio(); }
    
    /*─······································································─*/
    
    inline ulong fileno_count(){ return _count_; }

    inline bool  fileno_ready(){ return _count_ < get_soft_fileno(); }
    
}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/
