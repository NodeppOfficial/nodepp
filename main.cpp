#include <nodepp/nodepp.h>
#include <nodepp/wait.h>

using namespace nodepp;

void onMain(){

    wait_t<string_t> event;
    ptr_t<int> x ( 0UL, 10 );

    event.add( "event", coroutine::add( COROUTINE(){
    coBegin

        while( *x >= 0 ){
            console::log( "hello world", *x );
        coDelay( 1000 ); *x -= 1; }

    coFinish
    }));

    while( !event.empty() ){ event.emit( "event" ); }

}