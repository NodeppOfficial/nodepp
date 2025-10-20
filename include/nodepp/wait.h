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

    struct DONE {  bool /*--*/ *out;
        function_t<bool,T,A...> clb;
    };

    struct NODE {
        char skip     = 1;
        queue_t<DONE> que;
    };  ptr_t<NODE>   obj;

public:

    /*----*/ wait_t() noexcept : obj( new NODE() ) {}
    virtual ~wait_t() noexcept { /*--------------*/ }

    /*─······································································─*/

    void* operator()( T val, function_t<void> func ) const noexcept { return on(val,func); }

    /*─······································································─*/

    void* once( T val, function_t<void,A...> func ) const noexcept {
        ptr_t<bool> out = new bool(1); DONE ctx;
        ctx.out=&out; ctx.clb=([=]( T arg, A... args ){
            if( val != arg ){ return true;   } /*------------------*/
            if(*out != 0   ){ func(args...); } /*------------------*/
            if( out.null() ){ return false;  } *out = 0; return *out;
        }); obj->que.push(ctx); return obj->que.last();
    }

    void* on( T val, function_t<void,A...> func ) const noexcept {
        ptr_t<bool> out = new bool(1); DONE ctx;
        ctx.out=&out; ctx.clb=([=]( T arg, A... args ){
            if( val != arg ){ return true;   } /*--------*/
            if(*out != 0   ){ func(args...); } /*--------*/
            if( out.null() ){ return false;  } return *out;
        }); obj->que.push(ctx); return obj->que.last();
    }

    void off( void* address ) const noexcept {
        auto node = obj->que.get(address);
        if ( node == nullptr ){ return; }
        /**/ obj->que.erase( node );
    }

    /*─······································································─*/

    bool  empty() const noexcept { return obj->que.empty(); }
    ulong  size() const noexcept { return obj->que.size (); }
    void  clear() const noexcept { /*--*/ obj->que.clear(); }

    /*─······································································─*/

    void emit( const T& arg, const A&... args ) const noexcept {
        if( obj.null() || is_paused() ){ return; } auto x=obj->que.first();
        while( x!=nullptr && !obj->que.empty() ) { auto y=x->next; auto z=x->data;
        if   ( *z.out == 0 ) /*---------------*/ { obj->que.erase( x ); }
        elif ( !z.clb(arg,args...) ) /*-------*/ { obj->que.erase( x ); }
    x=y; }}

    /*─······································································─*/

    bool is_paused() const noexcept { return obj->skip<=0; }

    void    resume() const noexcept { obj->skip = 1; }

    void      stop() const noexcept { obj->skip = 0; }

    void      skip() const noexcept { obj->skip =-1; }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
