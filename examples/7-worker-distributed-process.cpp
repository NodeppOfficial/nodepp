#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/atomic.h>

using namespace nodepp;

void onWorker( int wid ){

    worker::add([=](){

        process::add( coroutine::add( COROUTINE(){
        coBegin

            console::log( "hello world", process::now(), wid );
            coDelay(1000);

        coFinish
        }));

    console::log( wid, "--->", process::size() );
    process::wait(); return -1; });

}

void onMain(){

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ 
        while( process::size() > 16 ){ coDelay(100); }
            console::log( "main --->", process::size() );
            onWorker( process::size() );
        coNext; }

    coFinish
    }));

}