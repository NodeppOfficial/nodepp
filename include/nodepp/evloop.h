/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EVENT_LOOP
#define NODEPP_EVENT_LOOP

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace /*anonimous*/ { class event_loop_t : public generator_t {
private:

    loop_t _loop_,_foop_; poll_t _poll_;
    bool _EXIT_= false; probe_t _probe_;

public:

    ulong size(){ return _loop_.size() + _poll_.size() + _foop_.size(); }

    void clear(){ _loop_.clear(); _poll_.clear(); _foop_.clear(); }

    bool empty(){ return size() <= 0; }

    /*─······································································─*/

    bool should_close(){ return _EXIT_ || empty(); }

    /*─······································································─*/

    void off( void* address ){ clear( address ); }

    void clear( void* address ){
         if( address == nullptr ){ return; }
         memset( address, 0, sizeof(bool) );
    }

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

    template< class T, class... V > int await( T cb, const V&... args ){ 
        int c=0;

        if ( !_EXIT_ && (c=cb(args...))>=0 ){
        if ( c==1 ){ auto t = coroutine::getno().delay;
        if ( t >0 ){ process::set_timeout( t ); }
        else /*-*/ { process::set_timeout(0UL); }} next(); return 1; } 
    
    return -1; }

    int next(){ probe_t tmp=_probe_; 
    ulong task= size()+ tmp.get()-1-_poll_.size(); coBegin
        
        while( !should_close() ){ 
        while( _loop_.next(/**/)>=0 ){ coNext; } process::set_timeout( _loop_.get_delay() ); 
        while( _foop_.next(/**/)>=0 ){ coNext; } process::set_timeout( _foop_.get_delay() );
        while( _poll_.next(task)>=0 ){ coNext; } coYield(1); }
    
    coFinish }

    /*─······································································─*/

    void exit( int err=0 ){ 
        if( should_close() ){ goto DONE; }
        _EXIT_=true; clear(); DONE:; ::exit(err); 
    }

};}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace /*anonimous*/ { event_loop_t& NODEPP_EV_LOOP(){
    thread_local static event_loop_t evloop; return evloop;
}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process { 

    template< class... T >
    void  await( const T&... args ){ while (NODEPP_EV_LOOP().await( args... )==1){ /*unused*/ } }

    template< class... T >
    void*  foop( const T&... args ){ return NODEPP_EV_LOOP().foop ( args... ); }

    template< class... T >
    void*  loop( const T&... args ){ return NODEPP_EV_LOOP().loop ( args... ); }

    template< class... T >
    void*  poll( const T&... args ){ return NODEPP_EV_LOOP().poll ( args... ); }

    template< class... T >
    void*  add ( const T&... args ){ return NODEPP_EV_LOOP().loop ( args... ); }
    
    /*─······································································─*/

    inline bool should_close(){ return NODEPP_EV_LOOP().should_close(); }
    inline bool        empty(){ return NODEPP_EV_LOOP().empty(); }
    inline ulong        size(){ return NODEPP_EV_LOOP().size (); }
    inline void        clear(){ /*--*/ NODEPP_EV_LOOP().clear(); }

    /*─······································································─*/

    inline void clear( void* address ){ NODEPP_EV_LOOP().off( address ); }
    inline void   off( void* address ){ NODEPP_EV_LOOP().off( address ); }
    inline void  exit( int err=0 )    { NODEPP_EV_LOOP().exit(err); }

    /*─······································································─*/

    inline int next(){ return NODEPP_EV_LOOP().next(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/