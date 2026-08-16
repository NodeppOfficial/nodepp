#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    ptr_t<int> x ( 0UL, 5 );

    worker::add( coroutine::add( COROUTINE(){
    coBegin

        while( *x > 0 ){
            console::log( "wrk2>> Hello World", *x );
        *x-=1; coDelay(3000); }

    coFinish
    }));

    /*
    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( *x > 0 ){
            console::log( "---" );
        coDelay(1000); }

    coFinish
    }));
    */

}