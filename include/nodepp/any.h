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
public: any_t() noexcept {};

    any_t( const char* f ) noexcept { set( string::to_string(f) ); }

    template< class T >
    any_t( const T& f ) noexcept { set( f ); }

   ~any_t() noexcept {}

    /*─······································································─*/

    uint type_size() const noexcept { return  any_sz.null()?0: *any_sz; }
    ulong    count() const noexcept { return  any_ptr.count(); }
    bool     empty() const noexcept { return  any_ptr.null (); }
    bool has_value() const noexcept { return !any_ptr.null (); }

    /*─······································································─*/

    void free() const noexcept { any_ptr.free(); }

    /*─······································································─*/

    void operator=( const char* f ) noexcept { set( string::to_string(f) ); }

    template< class T >
    void operator=( const T& f ) noexcept { set( f ); }

    template< class T >
    T as() const { return get<T>(); }

    template< class T >
    void set( const T& f ) noexcept {
        any_sz  = new uint(sizeof(T));
        any_ptr = new any_impl<T>(f);
    }

    template< class T >
    T get() const {
        char any [ sizeof(T)/sizeof(char) ]; if( !has_value() )
          { throw except_t("any_t is null"); }  
        if( *any_sz != sizeof(any)*sizeof(char) )
          { throw except_t("any_t incompatible sizetype"); }
        any_ptr->get((void*)&any); return *(T*)(any);
    }

    /*─······································································─*/

    template< class T >
    explicit operator T(void) const noexcept { return get<T>(); }

private:

    class any_base {
    public:
        virtual ~any_base() noexcept {}
        virtual void get( void* /*unused*/ ) const noexcept {}
        virtual void set( void* /*unused*/ )       noexcept {}
    };

    /*─······································································─*/

    template< class T >
    class any_impl : public any_base {
    public:
        any_impl( const T& f ) noexcept : any( f ) {}
        virtual void get( void* argc ) const noexcept { memcpy( argc, (void*)&any, sizeof(T) ); }
        virtual void set( void* argc )       noexcept { memcpy( (void*)&any, argc, sizeof(T) ); }
    private:
        T any;
    };

    /*─······································································─*/

    ptr_t<any_base> any_ptr;
    ptr_t<uint>     any_sz;

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
