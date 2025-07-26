#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    ptr_t<int> x = new int(100);
    mutex_t mut;

    worker::add( coroutine::add( COROUTINE(){
    coBegin

        while( *x > 0 ){ mut.emit([&](){
            console::log( "wrk2>> Hello World", *x );
        *x-=1; }); coDelay(100); }

    coFinish
    }));

    worker::add( coroutine::add( COROUTINE(){
    coBegin

        while( *x > 0 ){ mut.emit([&](){
            console::log( "wrk1>> Hello World", *x );
        *x-=1; }); coDelay(100); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( *x > 0 ){ mut.emit([&](){
            console::log( "task>> Hello World", *x );
        *x-=1; }); coDelay(100); }

    coFinish
    }));

}