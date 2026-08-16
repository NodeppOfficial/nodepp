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

    template< class U >
    function_t( const U& value ) noexcept : func_ptr( new func_impl<U>( value ) ) {}

    function_t( null_t ) noexcept : func_ptr(nullptr) {}

    function_t() noexcept : func_ptr(nullptr) {}
    
    /*─······································································─*/

    explicit operator bool(void) const noexcept { return func_ptr.null(); }
    
    /*─······································································─*/

    bool has_value() const noexcept { return func_ptr.has_value(); }
    ulong    count() const noexcept { return func_ptr.count(); }
    bool     empty() const noexcept { return func_ptr.null (); }
    bool      null() const noexcept { return func_ptr.null (); }
    void      free() const noexcept { /*--*/ func_ptr.free (); }
    void     clear() const noexcept { /*--*/ func_ptr.free (); }
    
    /*─······································································─*/
    
    template< typename U = V >
    typename type::enable_if< type::is_same<U,void>::value, U >::type
    operator()( const T&... arg ) const { emit( arg... ); }
    
    template< typename U = V >
    typename type::enable_if< type::is_same<U,void>::value, U >::type
    emit( const T&... arg ) const { if( has_value() ){ 
        func_ptr->invoke ( (void*) nullptr, arg... ); 
    }}
    
    /*─······································································─*/
    
    template< typename U = V >
    typename type::enable_if< !type::is_same<U,void>::value, U >::type
    operator()( const T&... arg ) const { return emit( arg... ); }
    
    template< typename U = V >
    typename type::enable_if< !type::is_same<U,void>::value, U >::type
    emit( const T&... arg ) const { U out; if( has_value() ){
        func_ptr->invoke ( &out, arg... );
    } return out; }
    
private:

    class func_base { public:
        virtual ~func_base () /*--*/ noexcept {}
        virtual void invoke( V*, const T&... ){}
    };
    
    /*─······································································─*/
    
    template< class F >
    class func_impl : public func_base { private:

        template< typename U = V >
        typename type::enable_if< !type::is_same<U,void>::value, void >::type
        invoker_helper( U* dst, const T&... args ){ *dst = callback( args... ); }

        template< typename U = V >
        typename type::enable_if< type::is_same<U,void>::value, void >::type
        invoker_helper( U*, const T&... args ){ callback( args... ); }

    public:

        virtual void invoke( V* out, const T&... arg ) override { 
            invoker_helper ( out, arg... );
        }

        func_impl( const F& value ) : callback( value ) {}

    private: F callback; };
    
    /*─······································································─*/
    
    ptr_t<func_base> func_ptr;
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/