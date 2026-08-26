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

    any_t( const char* value ) noexcept : any_ptr( new any_impl<string_t>( value ) ) {}

    template< class T >
    any_t( const T& value ) noexcept : any_ptr( new any_impl<T>( value ) ) {}

    any_t( null_t ) noexcept {}
    any_t()         noexcept {}

    /*─······································································─*/

    ulong type_size() const noexcept { return  empty() ?0 : any_ptr->size(); }
    ulong     count() const noexcept { return  any_ptr.count(); } /*--------*/
    bool      empty() const noexcept { return  any_ptr.null (); } /*--------*/
    bool  has_value() const noexcept { return !any_ptr.null (); } /*--------*/
    void       free() const noexcept { /*--*/  any_ptr.free (); } /*--------*/

    /*─······································································─*/

    template< class T > 
    explicit operator T(void) const noexcept { return as<T>(); }

    /*─······································································─*/

    template< typename T >
    typename type::enable_if< !type::is_same<T,any_t>::value, bool >::type
    is() const noexcept { return empty() ? false : type_size()==sizeof(T); }

    template< typename T >
    typename type::enable_if< !type::is_same<T,any_t>::value, T >::type
    as() const {
    if( !is<T>() ){ NODEPP_THROW_ERROR("any_t invalid value"); }
    return *type::cast<T>( raw() ); }

    /*─······································································─*/

    void* raw() const noexcept { 
    void* ptr = nullptr; any_ptr->get( ptr ); return ptr; }

    /*─······································································─*/

    template< typename T >
    typename type::enable_if< type::is_same<T,any_t>::value, bool >::type
    is() const noexcept { return !empty(); }

    template< typename T >
    typename type::enable_if< type::is_same<T,any_t>::value, any_t >::type
    as() const { return *this; }

private:

    class any_base {
    public:
        virtual ~any_base () /*---*/ noexcept {}
        virtual void  get ( void*& ) noexcept {}
        virtual ulong size() /*---*/ noexcept =0;
    };

    /*─······································································─*/

    template< class T >
    class any_impl : public any_base {
    public:
        virtual void  get ( void*& argc ) noexcept override { argc = (void*) &any; }
        virtual ulong size() /*--------*/ noexcept override { return sizeof(T); }
        any_impl( const T& value ) /*--*/ noexcept :any( type::bind<T>( value ) ) {}
    private: ptr_t<T> any; };

    /*─······································································─*/

    ptr_t<any_base> any_ptr;

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/