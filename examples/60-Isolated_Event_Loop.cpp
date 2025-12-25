#include <nodepp/nodepp.h>
#include <nodepp/loop.h>

using namespace nodepp;

void onMain(){

    loop_t evloop;

    evloop.add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "-> task 1" );
        coDelay(100); }

    coFinish
    }));

    evloop.add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "-> task 2" );
        coDelay(1000); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while  ( !evloop.empty()    ){           // block the coroutine
        while  ( evloop.next()>=0   ){ coNext; } // execute all task asynchronously
        coDelay( evloop.get_delay() ); }         // delay the task using the lowest timer detected by evloop

    coFinish
    }));

}