/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_POPEN
#define NODEPP_POPEN

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
#define PWROUTINE( str, cb ) worker::add( cb )
#define PWDELAY( TIMEOUT ) worker::delay( TIMEOUT )

    #include "fs.h"
    #include "worker.h"
    #include "promise.h"
    #include "initializer.h"
    #include "windows/popen.h"

#elif _KERNEL_ == NODEPP_KERNEL_POSIX
#define PWROUTINE( str, cb ) process::poll( str, POLL_STATE::READ | POLL_STATE::EDGE, cb )
#define PWDELAY( TIMEOUT )

    #include "fs.h"
    #include "promise.h"
    #include "initializer.h"
    #include "posix/popen.h"

#else
    #error "This OS Does not support popen.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace popen {

    template< class... T > 
    expected_t<popen_t,except_t> add( const T&... args ){ try {

        auto rd1 = type::bind( generator::file::read() );
        auto rd2 = type::bind( generator::file::read() );
        auto pid = type::bind( popen_t( args... ) );

    PWROUTINE( pid->std_output(), coroutine::add( COROUTINE(){
    coBegin

        if( (*rd1)( &pid->std_output() )==1 ){ coGoto(0); }
        if( rd1->state==0 )/*-*/{ pid->free(); coEnd; }
        pid->onDout.emit( rd1->data );
        pid->onData.emit( rd1->data );
    
    PWDELAY( 100 );
    coGoto(0); coFinish; }));

    PWROUTINE( pid->std_error(), coroutine::add( COROUTINE(){
    coBegin

        if( (*rd2)( &pid->std_error() )==1 ){ coGoto(0); }
        if( rd2->state==0 )/**/{ pid->free(); coEnd; }
        pid->onDerr.emit( rd2->data );
        pid->onData.emit( rd2->data );

    PWDELAY( 100 );
    coGoto(0); coFinish; }));

        return *pid; 
    
    } catch( except_t err ) { return err; } }

/*─······································································─*/
    
    template< class... T > 
    promise_t<string_t,except_t> resolve( const T&... args ){
    return promise_t<string_t,except_t>([=]( 
           res_t<string_t> res, rej_t<except_t> rej 
    ){

        auto pid = popen::add( args... );
        auto bff = ptr_t<string_t>( 0UL );

    if( !pid.has_value() ){ rej( pid.error() ); }

        pid.value().onDrain([=](){ res( *bff ); });
        pid.value().onData([=]( string_t chunk ){ 
            *bff += chunk; 
        });

    }); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef PWROUTINE
#undef PWDELAY
#endif

/*────────────────────────────────────────────────────────────────────────────*/