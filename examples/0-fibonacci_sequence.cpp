#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain() {

    ptr_t<ullong> x ( 2UL, 1UL ); //--> ( buffer_size, initial_value ) - 

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ do {
            ullong tmp = x[0];
            x[0] += x[1];
            x[1]  = tmp;
            console::log( ">>", x[0] ); 
        } while(0); coDelay(1000); }

    coFinish
    }));

}
