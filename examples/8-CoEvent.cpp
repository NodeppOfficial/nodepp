#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

void onMain(){

    event_t<> event;
    ptr_t<int> x ( 0UL, 10 );

    event.add( coroutine::add( COROUTINE(){
    coBegin

        while( *x >= 0 ){
            console::log( "hello world", *x );
        coDelay( 1000 ); *x -= 1; }

    coFinish
    }));

    while( !event.empty() ){ event.emit(); }

}