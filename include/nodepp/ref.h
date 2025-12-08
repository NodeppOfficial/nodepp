/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_REF
#define NODEPP_REF

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T > class ref_t {
public:

    ref_t( ptr_t<T> value ) noexcept : address( new ptr_t<T> ){ *address=value; }
    ref_t() /*-----------*/ noexcept : address( new ptr_t<T> ){}
    ref_t( null_t ) /*---*/ noexcept {}
    virtual ~ref_t() /*--*/ noexcept {}

    /*─······································································─*/

    bool operator> ( ref_t& oth ) const noexcept { return data()> oth.data(); }
    bool operator>=( ref_t& oth ) const noexcept { return data()>=oth.data(); }
    bool operator< ( ref_t& oth ) const noexcept { return data()< oth.data(); }
    bool operator<=( ref_t& oth ) const noexcept { return data()<=oth.data(); }
    bool operator==( ref_t& oth ) const noexcept { return data()==oth.data(); }
    bool operator!=( ref_t& oth ) const noexcept { return data()!=oth.data(); }

    /*─······································································─*/

    bool operator> ( T* value ) const noexcept { return data()> value; }
    bool operator>=( T* value ) const noexcept { return data()>=value; }
    bool operator< ( T* value ) const noexcept { return data()< value; }
    bool operator<=( T* value ) const noexcept { return data()<=value; }
    bool operator==( T* value ) const noexcept { return data()==value; }
    bool operator!=( T* value ) const noexcept { return data()!=value; }

    /*─······································································─*/

    T& operator[]( ulong i ) const noexcept {
       return size() !=0 ? data()[i%size()] 
       /*-------------*/ : data()[i]; 
    }

    /*─······································································─*/

    ulong    count() const noexcept { return null() ? 0 /*-*/ : address->count(); }
    ulong     size() const noexcept { return null() ? 0 /*-*/ : address->size(); }

    T*        data() const noexcept { return null() ? nullptr : address->data(); }
    T*         get() const noexcept { return null() ? nullptr : address->data(); }
    T*       begin() const noexcept { return null() ? nullptr : address->data(); }
    T*         end() const noexcept { return null() ? nullptr : address->end();  }

    bool     empty() const noexcept { return  null() || address->empty(); }
    bool has_value() const noexcept { return !null() && address->has_value(); }
    bool      null() const noexcept { return address.null() || address->null(); }

    /*─······································································─*/

    void clear() noexcept { address->clear(); }
    void reset() noexcept { address->reset(); }
    void free () noexcept { address->free (); }

    /*─······································································─*/

    explicit operator bool(void) const noexcept { return  has_value(); }
    explicit operator   T*(void) const /*----*/ { return  data(); }

    T* operator->() /*--------*/ const noexcept { return  data(); }
    T& operator* () /*--------*/ const noexcept { return *data(); }
    T* operator& () /*--------*/ const noexcept { return  data(); }

protected:

    ptr_t<ptr_t<T>> address;

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif