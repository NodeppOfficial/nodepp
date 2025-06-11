/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TASK
#define NODEPP_TASK

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { 
   struct nodepp_process_waiter { bool blk; bool out; }; 
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

namespace task {

    queue_t<function_t<int>> queue;

    void clear(){        queue.clear(); }
    ulong size(){ return queue.size (); }
    bool empty(){ return queue.empty(); }

    void clear( void* address ){
       if( address == nullptr ){ return; }
        *((bool*)( address )) = 0;
    }

    template< class T, class... V >
    void* add( T cb, const V&... arg ){
        if( MAX_TASKS!=0 && queue.size()>=MAX_TASKS ){ return nullptr; }
        ptr_t<nodepp_process_waiter> obj = new nodepp_process_waiter();
        obj->blk=0; obj->out=1; auto clb = type::bind(cb);
        queue.push([=](){
            if( obj->out==0 ){ return -1; }
            if( obj->blk==1 ){ return  1; } 
                obj->blk =1; int rs=(*clb)( arg... );
            if( clb.null()  ){ return -1; }  
                obj->blk =0;   return !obj->out?-1:rs;
        }); return (void*) &obj->out;
    }

    bool next(){
        if( queue.empty() ){ return 0; }
            auto x = queue.get();
            int  y = x->data  ();
        if( y==-1 ){ queue.erase(x); }
        if( y== 1 ){ queue.next();   } return 1;
    }

}

    /*─······································································─*/

namespace loop {

    queue_t<function_t<int>> queue;

    void clear(){        queue.clear(); }
    ulong size(){ return queue.size (); }
    bool empty(){ return queue.empty(); }

    void clear( void* address ){
       if( address == nullptr ){ return; }
        *((bool*)( address )) = 0;
    }

    template< class T, class... V >
    void* add( T cb, const V&... arg ){
        if( MAX_TASKS!=0 && queue.size()>=MAX_TASKS ){ return nullptr; }
        ptr_t<nodepp_process_waiter> obj = new nodepp_process_waiter();
        obj->blk=0; obj->out=1; auto clb = type::bind(cb);
        queue.push([=](){
            if( obj->out==0 ){ return -1; }
            if( obj->blk==1 ){ return  1; } 
                obj->blk =1; int rs=(*clb)( arg... );
            if( clb.null()  ){ return -1; }  
                obj->blk =0;   return !obj->out?-1:rs;
        }); return (void*) &obj->out;
    }

    bool next(){
        if( queue.empty() ){ return 0; }
            auto x = queue.get();
            int  y = x->data  ();
        if( y==-1 ){ queue.erase(x); }
        if( y== 1 ){ queue.next();   } return 1;
    }

}

    /*─······································································─*/

namespace poll {

    queue_t<function_t<int>> queue;

    void clear(){        queue.clear(); }
    ulong size(){ return queue.size (); }
    bool empty(){ return queue.empty(); }

    void clear( void* address ){
       if( address == nullptr ){ return; }
        *((bool*)( address )) = 0;
    }

    template< class T, class... V >
    void* add( T cb, const V&... arg ){
        if( MAX_TASKS!=0 && queue.size()>=MAX_TASKS ){ return nullptr; }
        ptr_t<nodepp_process_waiter> obj = new nodepp_process_waiter();
        obj->blk=0; obj->out=1; auto clb = type::bind(cb);
        queue.push([=](){
            if( obj->out==0 ){ return -1; }
            if( obj->blk==1 ){ return  1; } 
                obj->blk =1; int rs=(*clb)( arg... );
            if( clb.null()  ){ return -1; }  
                obj->blk =0;   return !obj->out?-1:rs;
        }); return (void*) &obj->out;
    }

    bool next(){
        if( queue.empty() ){ return 0; }
            auto x = queue.get();
            int  y = x->data  ();
        if( y==-1 ){ queue.erase(x); }
        if( y== 1 ){ queue.next();   } return 1;
    }

}

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    int threads = 0;

    /*─······································································─*/

    void clear(){
        process::task::clear();
        process::poll::clear();
        process::loop::clear();
        process::threads = 0;
    }

    /*─······································································─*/

    void clear( void* address ){
        if( address == nullptr ){ return; }
        *((bool*)( address )) = 0;
    }

    /*─······································································─*/

    bool empty(){ return (
        process::task::empty() &&
        process::poll::empty() &&
        process::loop::empty() &&
        process::threads <= 0
    );}

    /*─······································································─*/

    ulong size(){
    return process::poll::size() +
           process::task::size() +
           process::loop::size() +
           process::threads      ;
    }

    /*─······································································─*/

    template< class... T >
    void* add( const T&... args ){ return process::loop::add( args... ); }

    /*─······································································─*/

    int next(){
    coStart

        if( !process::poll::empty() ){ process::poll::next(); coNext; }
        if( !process::loop::empty() ){ process::loop::next(); coNext; }
        if( !process::task::empty() ){ process::task::next(); coNext; }
             process::yield();

    coStop
    }

    /*─······································································─*/

    template< class T, class... V >
    void await( T cb, const V&... args ){ while( cb( args... )!=-1 ){ next(); } }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
