/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_NODEPP
#define NODEPP_NODEPP

/*────────────────────────────────────────────────────────────────────────────*/

#include "import.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { 

    /*─······································································─*/
    
    loop_t _loop_; poll_t _poll_; loop_t _foop_;

    /*─······································································─*/

    ulong size(){ return _TASK_ + _loop_.size() + _poll_.size() + _foop_.size(); }

    void clear(){ _TASK_=0; _loop_.clear(); _poll_.clear(); _foop_.clear(); }

    bool empty(){ return size() <= 0; }

    /*─······································································─*/

    void exit( int err=0 ){ _EXIT_=true; clear(); ::exit(err); }

    bool should_close(){ return _EXIT_ || empty(); }

    void clear( void* address ){
         if( address == nullptr ){ return; }
         memset( address, 0, sizeof(bool) );
    }

    /*─······································································─*/

    int next(){ coStart
        if( !_poll_.empty() ) { _poll_.next(); coNext; } /*----*/
        if( !_loop_.empty() ) { _loop_.next(); coNext; } /*----*/
        if( !_foop_.empty() ) { _foop_.next(); coNext; } yield();
    coStop }

    /*─······································································─*/

    template< class... T >
    void* foop( const T&... args ){ return _foop_.add( args... ); }

    template< class... T >
    void* loop( const T&... args ){ return _loop_.add( args... ); }

    template< class... T >
    void* poll( const T&... args ){ return _poll_.add( args... ); }

    template< class... T >
    void* add ( const T&... args ){ return _loop_.add( args... ); }

    /*─······································································─*/

    template< class T, class... V >
    void await( T cb, const V&... args ){ ++_TASK_;
         while( cb( args... )>=0 && !should_close() )
              { process::next(); } 
    --_TASK_; }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#include "env.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { array_t<string_t> args;

    template< class... T >
    void error( const T&... msg ){ throw except_t( msg... ); }

    /*─······································································─*/

    void start( int argc, char** args ){
        int i=0; onSIGEXIT.once([=](){ process::exit(1); }); do {
        if(!regex::test(args[i],"^\\?") ) {
            process::args.push(args[i]);
        } else {
            for( auto &x: query::parse( args[i] ).data() )
               { env::set( x.first, x.second ); }
        }
    } while( i ++< argc - 1 ); signal::start(); }

    /*─······································································─*/

    void stop(){ 
        while(!process::should_close() )
             { process::next(); }
        process::exit(1);
    }

    /*─······································································─*/

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
