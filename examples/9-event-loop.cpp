#include <nodepp/nodepp.h>

using namespace nodepp;

int main( int args, char** argc ) {

    loop_t event_loop;

    event_loop.add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "hello world", process::now() );
        coDelay(1000); }

    coFinish
    }));

    while( !event_loop.empty() ){
        /*-------*/ event_loop.next();
    process::delay( event_loop.get_delay() ); }

    return 0;
}