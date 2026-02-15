#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void main_thread_balancer() {

    int x=100000/os::cpus(); while( x-->0 ){ 

    timer::interval([=](){
        console::log( "hello world!", x );
    },10);

    }

}

void onMain() {

    for( int x=os::cpus(); x-->0; ){ worker::add([=](){
        main_thread_balancer();
        process::wait();
    return -1; }); }

}