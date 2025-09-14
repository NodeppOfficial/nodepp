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

    virtual ~any_t() noexcept {}

    /*----*/ any_t() noexcept {}

    /*─······································································─*/

    ulong type_size() const noexcept { return  empty() ?0 : any_ptr->size(); }
    ulong     count() const noexcept { return  any_ptr.count(); } /*--------*/
    bool      empty() const noexcept { return  any_ptr.null (); } /*--------*/
    bool  has_value() const noexcept { return !any_ptr.null (); } /*--------*/
    void       free() const noexcept { /*--*/  any_ptr.free (); } /*--------*/

    /*─······································································─*/

    template< class T >
    void set( const T& f ) noexcept { any_ptr = new any_impl<T>(f); }

    template< class T >
    T as() const { return get<T>(); }

    template< class T >
    T get() const {

        if( !has_value() ) /*----*/ { throw except_t("any_t is null"); } /*---------*/
        if( type_size()!=sizeof(T) ){ throw except_t("any_t incompatible sizetype"); }

        const ulong size = sizeof(T) / sizeof(char);
        char any[ size ]; any_ptr->get((void*)&any);
        return *(T*)(any); /*---------------------*/

    }

    /*─······································································─*/

    template< class T >
    explicit operator T(void) const noexcept { return get<T>(); }

private:

    class any_base {
    public:
        virtual ~any_base() noexcept {}
        virtual void  get( void* /*unused*/ ) const noexcept {}
        virtual void  set( void* /*unused*/ ) /*-*/ noexcept {}
        virtual ulong size() /*------------*/ const noexcept =0;
    };

    /*─······································································─*/

    template< class T >
    class any_impl : public any_base {
    public:
        any_impl( const T& f ) noexcept : any( type::bind(f) ) {}
        virtual ulong size() /*------*/ const noexcept { return any.null(/**/) ?0 : sizeof(T)  ; }
        virtual void  get( void* argc ) const noexcept { memcpy( argc, (void*)&any, sizeof(T) ); }
        virtual void  set( void* argc ) /*-*/ noexcept { memcpy( (void*)&any, argc, sizeof(T) ); }
    private:
        ptr_t<T> any;
    };

    /*─······································································─*/

    ptr_t<any_base> any_ptr;

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
