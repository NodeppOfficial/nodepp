/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TASK
#define NODEPP_TASK

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

namespace task {

    queue_t<function_t<int>> queue;

    void clear(){ queue.clear(); }

    ulong size(){ return queue.size(); }

    bool empty(){ return queue.empty(); }

    void clear( void* address ){ 
        if( address == nullptr ){ return; }
            *((bool*)( address )) = 0; 
    }

    template< class T, class... V >
    void* add( T cb, const V&... arg ){ 
        if( queue.size() >= MAX_TASKS ){ return nullptr; }
        ptr_t<T>    clb = new T( cb );
        ptr_t<bool> blk = new bool(0);
        ptr_t<bool> out = new bool(1); queue.push([=](){ 
            if( *out==0 ){ return -1; }
            if( *blk==1 ){ return  1; } *blk = 1;
            int rs = (*clb) (arg...);   *blk = 0; 
            return *out==0 ? -1 : rs; 
        }); return (void*) &out;
    }

    bool next(){
        if( queue.empty() ){ return 0; }
          auto x = queue.get();
          int  y = x->data();
        if( y ==-1 ){ queue.erase(x); }
        if( y == 1 ){ queue.next();   }
        return queue.get()!=queue.last();
    }

}

    /*─······································································─*/

namespace loop {

    queue_t<function_t<int>> queue;

    void clear(){ queue.clear(); }

    ulong size(){ return queue.size(); }

    bool empty(){ return queue.empty(); }

    void clear( void* address ){ 
        if( address == nullptr ){ return; }
            *((bool*)( address )) = 0; 
    }

    template< class T, class... V >
    void* add( T cb, const V&... arg ){ 
        if( queue.size() >= MAX_TASKS ){ return nullptr; }
        ptr_t<T>    clb = new T( cb );
        ptr_t<bool> blk = new bool(0);
        ptr_t<bool> out = new bool(1); queue.push([=](){ 
            if( *out==0 ){ return -1; }
            if( *blk==1 ){ return  1; } *blk = 1;
            int rs = (*clb) (arg...);   *blk = 0; 
            return *out==0 ? -1 : rs; 
        }); return (void*) &out;
    }

    bool next(){
        if( queue.empty() ){ return 0; }
          auto x = queue.get();
          int  y = x->data();
        if( y ==-1 ){ queue.erase(x); }
        if( y == 1 ){ queue.next();   }
        return queue.get()!=queue.last();
    }

}

    /*─······································································─*/

namespace poll {

    queue_t<function_t<int>> queue;

    void clear(){ queue.clear(); }

    ulong size(){ return queue.size(); }

    bool empty(){ return queue.empty(); }

    void clear( void* address ){ 
        if( address == nullptr ){ return; }
            *((bool*)( address )) = 0; 
    }

    template< class T, class... V >
    void* add( T cb, const V&... arg ){ 
        if( queue.size() >= MAX_FILENO ){ return nullptr; }
        ptr_t<T>    clb = new T( cb );
        ptr_t<bool> blk = new bool(0);
        ptr_t<bool> out = new bool(1); queue.push([=](){ 
            if( *out==0 ){ return -1; }
            if( *blk==1 ){ return  1; } *blk = 1;
            int rs = (*clb) (arg...);   *blk = 0; 
            return *out==0 ? -1 : rs; 
        }); return (void*) &out;
    }

    bool next(){
        if( queue.empty() ){ return 0; }
          auto x = queue.get();
          int  y = x->data();
        if( y ==-1 ){ queue.erase(x); }
        if( y == 1 ){ queue.next();   }
        return queue.get()!=queue.last();
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
        
        if( !process::loop::empty() ){ process::loop::next(); coNext; }
        if( !process::task::empty() ){ process::task::next(); coNext; }
        if( !process::poll::empty() ){ process::poll::next(); coNext; }
             process::yield();

    coStop
    }

    /*─······································································─*/

    template< class T, class... V > 
    void await( T cb, const V&... args ){ while( cb( args... )!=-1 ){ next(); } }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
