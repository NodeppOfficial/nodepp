#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/atomic.h>

using namespace nodepp;

atomic_t<int> shared_variable = 100;

GENERATOR( my_coroutine ){

    coEmit(){
    coBegin
        while( shared_variable-->0 ){
            console::log( "task>> Hello World", shared_variable.get() );
        coDelay(100); }
    coFinish
    }

};

void onMain(){

    // Launching a worker to run the coroutine
    worker::add(my_coroutine());
    // Launching another worker
    worker::add(my_coroutine());
    // Adding the coroutine to the main event loop as well
    process::add(my_coroutine());

}