#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    worker_t worker = worker::add( coroutine::add( COROUTINE(){
        static int x = 0;
    coBegin

        while( true ){
            console::log( "wrk>> Hello World", x );
        x++; coDelay(1000); }

    coFinish
    }) );

    process::add( coroutine::add( COROUTINE(){
    coBegin

        coDelay( 10000 ); worker.close();

    coFinish
    }) );

}