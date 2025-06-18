/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_WAIT
#define NODEPP_WAIT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class T, class... A > class wait_t { 
protected:

    struct NODE {  bool        *out;
        function_t<bool,T,A...> clb;
    };  queue_t<NODE> obj;

public:

    wait_t() noexcept {} ~wait_t() noexcept { free(); }
    
    /*─······································································─*/

    void* operator()( T val, function_t<void> func ) const noexcept { return on(val,func); }
    
    /*─······································································─*/

    void off( void* address ) const noexcept { process::clear( address ); }

    void* once( T val, function_t<void,A...> func ) const noexcept {
        if( MAX_EVENTS!=0 && obj.size()>=MAX_EVENTS ){ return nullptr; }
        ptr_t<bool> out = new bool(1); NODE ctx;
        ctx.out=&out; ctx.clb=([=]( T arg, A... args ){
            if( val == arg ){ return true;   }
            if(*out != 0   ){ func(args...); }
            if( out.null() ){ return false;  } *out = 0; return *out;
        }); obj.push(ctx); return &out;
    }

    void* on( T val, function_t<void,A...> func ) const noexcept {
        if( MAX_EVENTS!=0 && obj.size()>=MAX_EVENTS ){ return nullptr; }
        ptr_t<bool> out = new bool(1); NODE ctx;
        ctx.out=&out; ctx.clb=([=]( T arg, A... args ){
            if( val == arg ){ return true;   }
            if(*out != 0   ){ func(args...); }
            if( out.null() ){ return false;  } return *out;
        }); obj.push(ctx); return &out;
    }
    
    /*─······································································─*/

    bool  empty() const noexcept { return obj.empty(); }
    ulong  size() const noexcept { return obj.size (); }

    /*─······································································─*/

    void free() const noexcept {
        auto x=obj.first(); while( x!=nullptr && !obj.empty() ){
        auto y=x->next; if( *x->data.out==0 ){ obj.erase(x); } x=y; }
    }

    void clear() const noexcept {
        auto x=obj.first(); while( x!=nullptr && !obj.empty() ){
        auto y=x->next; *x->data.out==0; x=y;
    }}
    
    /*─······································································─*/

    void emit( const T& arg, const A&... args ) const noexcept {
        auto x=obj.first(); while( x!=nullptr && !obj.empty() ){
        auto y=x->next; 
            if( *x->data.out == 0 )        { x=y; continue; }
            if( !x->data.clb(arg,args...) ){ x=y; continue; } x=y; }
    }
    
};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif