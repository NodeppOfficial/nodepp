#include <nodepp/nodepp.h>
#include <nodepp/worker.h>

using namespace nodepp;

void onMain(){

    for( auto x=2; x-->0; ){ worker::add(
    coroutine::add( COROUTINE(){
    coBegin

        while( true ){ do {
            console::log( x, ">>", os::rand() ); // thread safe real random numbers
        } while(0); coDelay(1000); }

    coFinish
    }) );}

}