#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

void onMain(){

    event_t<> event;
    ptr_t<int> x ( 0UL, 10 );

    event.add( coroutine::add( COROUTINE(){
    coBegin

        while( *x >= 3 ){
            console::log( "hello world A", *x );
        coNext; *x -= 1; }

    coFinish
    }));

    event.add( coroutine::add( COROUTINE(){
    coBegin

        while( *x >= 2 ){
            console::log( "hello world B", *x );
        coNext; *x -= 1; }

    coFinish
    }));

    event.add( coroutine::add( COROUTINE(){
    coBegin

        while( *x >= 1 ){
            console::log( "hello world C", *x );
        coNext; *x -= 1; }

    coFinish
    }));

    while( !event.empty() ){ 
        event.emit(); 
        process::delay(1000);
        console::log( "---", event.size() );
    }

}