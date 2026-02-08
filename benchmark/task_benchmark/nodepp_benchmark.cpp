#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain() {

    int x=100000; while( x-->0 ){ 

    timer::interval([=](){
        console::log( "hello world!", x );
    },10);

    }

}