/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_FUNCTION
#define NODEPP_FUNCTION

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class V, class... T > class function_t {
public:
    
    template< class F >
    function_t( F f ) : func_ptr( new func_impl<F>(f) ) {}
   
    function_t() noexcept : func_ptr(nullptr) {}
    
    virtual ~function_t() noexcept {}
    
    /*─······································································─*/

    bool has_value() const noexcept { return func_ptr.has_value(); }
    ulong    count() const noexcept { return func_ptr.count(); }
    bool     empty() const noexcept { return func_ptr.null (); }
    bool      null() const noexcept { return func_ptr.null (); }
    void      free() const noexcept { /*--*/ func_ptr.free (); }
    
    /*─······································································─*/

    explicit operator bool(void)    const noexcept { return func_ptr.null(); }
    
    V operator()( const T&... arg ) const /*----*/ { return emit( arg... );  }
    
    V emit( const T&... arg ) const { 
        if( !has_value() ){ return V(); }
        return func_ptr->invoke( arg... ); 
    }
    
private:

    class func_base { public:
        virtual ~func_base() { /*-----------------*/ }
        virtual V invoke( const T&... arg ) const = 0;
    };
    
    /*─······································································─*/
    
    template< class F >
    class func_impl : public func_base {

    public:

        func_impl( const F& f ) : func(f) { /*------------------------*/ }
        virtual V invoke( const T&... arg ) const { return func(arg...); }

    private:
        F func;
    };
    
    /*─······································································─*/
    
    ptr_t<func_base> func_ptr;
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
