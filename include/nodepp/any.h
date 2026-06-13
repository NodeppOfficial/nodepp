/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ANY
#define NODEPP_ANY

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class any_t {
public:

    any_t( const char* f ) noexcept { set( string::to_string(f) ); }

    any_t( null_t ) noexcept { /*---------*/ }

    template< class T >
    any_t( const T& f ) noexcept { set( f ); }

    any_t() noexcept {}

    /*─······································································─*/

    ulong type_size() const noexcept { return  empty() ?0 : any_ptr->size(); }
    ulong     count() const noexcept { return  any_ptr.count(); } /*--------*/
    bool      empty() const noexcept { return  any_ptr.null (); } /*--------*/
    bool  has_value() const noexcept { return !any_ptr.null (); } /*--------*/
    void       free() const noexcept { /*--*/  any_ptr.free (); } /*--------*/

    /*─······································································─*/

    template< class T > explicit operator T(void) const noexcept { return get<T>(); }

    template< class T >
    void set( const T& f ) noexcept { any_ptr = new any_impl<T>(f); }

    template< class T >
    bool is() const noexcept { return type_size()==sizeof(T); }

    template< class T >
    T& as() const { return get<T>(); }

    template< class T >
    T& get() const { void* ptr = nullptr; any_ptr->ptr( ptr ); 

        if( ptr==nullptr ){ NODEPP_THROW_ERROR("any_t is null"); }
        if( !is<T>() )/**/{ NODEPP_THROW_ERROR("any_t incompatible sizetype"); }

    return * type::cast<T>(ptr); }

    /*─······································································─*/

private:

    class any_base {
    public:
        virtual ~any_base () /*---------*/ noexcept {}
        virtual void  ptr ( void*& ) const noexcept {}
        virtual ulong size() /*---*/ const noexcept =0;
    };

    /*─······································································─*/

    template< class T >
    class any_impl : public any_base {
    public:
        any_impl( const T& f ) noexcept : any( type::bind(f) ) {}
        virtual ulong size() /*-------*/ const noexcept { return any.null() ?0 : sizeof(T)  ; }
        virtual void  ptr( void*& argc ) const noexcept { argc = &any; }
    private:
        ptr_t<T> any;
    };

    /*─······································································─*/

    ptr_t<any_base> any_ptr;

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
