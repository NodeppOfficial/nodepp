#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/atomic.h>

using namespace nodepp;

void onWorker(){

    worker::add([=](){

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( true ){
                console::log( "hello world", (void*) worker::pid() );
            coDelay(1000); process::clear(); }

        coFinish
        }));

    process::wait(); return -1; });

}

void onMain(){

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ 
        while( process::size() > 16 ){ coDelay(100); }
            console::log( "--->", process::size() );
            onWorker();
        coNext; }

    coFinish
    }));

}