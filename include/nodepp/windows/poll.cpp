/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#pragma once
#include <winsock.h>
#include <winsock2.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { enum POLL_STATE {
    READ = 1, WRITE = 2, DUPLEX = 3
};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using POLLFD = struct pollfd; }
namespace nodepp { class poll_t : public generator_t {
private:
    
    using NODE_CLB = function_t<int>;
    
    struct waiter { bool blk ; bool out; }; 

protected:

    struct NODE {
        array_t<NODE_CLB> clb;
        array_t<POLLFD> mem;
        int        y,len;
    };  ptr_t<NODE>  obj;

    /*─······································································─*/
    
    template< class T, class... V >
    void* listen( const ulong fd, const int flag, T& clb, const V&... args ) noexcept {
        if( flag & POLL_STATE::READ  ){ return push( fd, POLLIN , clb, args... ); }
      elif( flag & POLL_STATE::WRITE ){ return push( fd, POLLOUT, clb, args... ); }
    return nullptr; }

    void remove( int idx ) const noexcept { 
         obj->clb.erase( idx ); 
         obj->mem.erase( idx );
    }

    /*─······································································─*/

    template< class T, class... V >
    void* push( ulong fd, int flag, T cb, const V&... arg ) const noexcept {

        ptr_t<waiter> tsk = new waiter();
        auto clb=type::bind( cb );
        tsk->blk=0; tsk->out=1;

        obj->mem.unshift({ fd, (uchar) flag, 0x00 });

        obj->clb.unshift([=](){
            if( tsk->out==0 ){ return -1; }
            if( tsk->blk==1 ){ return  1; } 
                tsk->blk =1; int rs=(*clb)( arg... );
            if( clb.null () ){ return -1; }  
                tsk->blk =0; return !tsk->out ? -1 : rs;
        });
        
        return (void*) &tsk->out;
    }

    /*─······································································─*/

    void clear( void* address ){
         if( address == nullptr ){ return; }
         memset( address, 0, sizeof(bool) );
    }

public:

    virtual ~poll_t() noexcept { if( obj.count()>1 ){ return; } }

    poll_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    void clear() const noexcept { /*--*/ obj->clb.clear(); obj->mem.clear(); }

    ulong size() const noexcept { return obj->clb.size (); }

    bool empty() const noexcept { return obj->clb.empty(); }

    /*─······································································─*/

    template< class T, class U, class... W >
    void* add( T& inp, uchar imode, U cb, const W&... args ) noexcept {
          return listen( inp.get_fd(), imode, cb, args... ); 
    }

    /*─······································································─*/

    int next() noexcept { 
    coBegin

        if((obj->len=::WSAPoll( obj->mem.data(), obj->mem.size(), 0 ))<=0 )
          { coEnd; } obj->y=obj->mem.last(); coNext;
        
        while( obj->y >= 0 && obj->len >= 0 ){
           if( obj->mem[ obj->y ].revents == 0 ){ --obj->y; continue; } 
        do   {

            if( obj->mem[ obj->y ].revents & ( POLLERR | POLLHUP ) &&
              ( obj->mem[ obj->y ].revents & ( POLLIN  | POLLOUT ))==0
            ) { remove( obj->y ); --obj->y; --obj->len; break; }
        
            switch( obj->clb[ obj->y ].emit() ){
                case -1: remove( obj->y ); --obj->y; --obj->len; break;
                case  1: /*-------------*/ --obj->y; --obj->len; break;
                default: /*-----------------------------------*/ break;
            }

        } while(0); coNext; }

    coFinish }

};}

/*────────────────────────────────────────────────────────────────────────────*/