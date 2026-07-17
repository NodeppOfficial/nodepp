#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/channel.h>
#include <nodepp/encoder.h>

using namespace nodepp;

#include "module1.h"
#include "module2.h"

void onMain(){

    pair_t<worker_t,channel_t<coroutine_t>> module2 = module_2_start();
    pair_t<worker_t,channel_t<coroutine_t>> module1 = module_1_start();

    module2.second.write( coroutine_t( COROUTINE(){
    coBegin

        while( true ){
            console::log( "hello from module 2" );
        coDelay(100); }

    coFinish
    }));

    module1.second.write( coroutine_t( COROUTINE(){
    coBegin

        while( true ){
            console::log( "hello from module 1" );
        coDelay(300); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "<---------------->" );
            console::log( "<2>", module2.first.is_closed() );
            console::log( "<2>", module1.first.is_closed() );
        coDelay(1000); }

    coFinish
    }));

}