/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT
#define NODEPP_EVENT

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { template< class... A > class event_t {
protected:

    struct NODE {  bool      *out;
        function_t<bool,A...> clb;
    };  queue_t<NODE> obj;

public:

    event_t() noexcept {} ~event_t() noexcept { free(); }

    /*─······································································─*/

    void* operator()( function_t<void,A...> func ) const noexcept { return on(func); }

    /*─······································································─*/

    void off( void* address ) const noexcept { process::clear( address ); }

    void* once( function_t<void,A...> func ) const noexcept {
        if( MAX_EVENTS!=0 && obj.size()>=MAX_EVENTS ){ return nullptr; }
        ptr_t<bool> out = new bool(1); NODE ctx;
        ctx.out=&out; ctx.clb=([=]( A... args ){
            if(*out != 0   ){ func(args...); }
            if( out.null() ){ return false;  } *out = 0; return *out;
        }); obj.push(ctx); return &out;
    }

    void* on( function_t<void,A...> func ) const noexcept {
        if( MAX_EVENTS!=0 && obj.size()>=MAX_EVENTS ){ return nullptr; }
        ptr_t<bool> out = new bool(1); NODE ctx;
        ctx.out=&out; ctx.clb=([=]( A... args ){
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

    void emit( const A&... args ) const noexcept {
        auto x=obj.first(); while( x!=nullptr && !obj.empty() ){
        auto y=x->next;
            if( *x->data.out == 0 )    { x=y; continue; }
          elif( !x->data.clb(args...) ){ x=y; continue; } x=y; }
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif