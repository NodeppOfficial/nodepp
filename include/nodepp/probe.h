/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_PROBE
#define NODEPP_PROBE

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class probe_t {
private:

    ptr_t<uchar> counter;

public:

    /*----*/ probe_t():counter( new uchar() ){}
    virtual ~probe_t() noexcept /*--------*/ {}

    void clear() /*-*/ noexcept { counter= new uchar(); }

    ulong  get() const noexcept { return counter.count()-1; }

}; }

/*────────────────────────────────────────────────────────────────────────────*/

#endif
