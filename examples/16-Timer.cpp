#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    // Interval: a setInterval equivalent
    auto A = timer::interval([=](){ 
        console::done("set interval");
    },1000);

    // Timeout: a setTimeout equivalent
    auto B = timer::timeout([=](){
        console::error("set timeout");
    },10000);

    // add: a Coroutine Based Timer
    auto C = timer::add([=](){
        static uchar x = 100;
    coStart

        while( x-->0 ){
            console::log(" timer task ");
            coNext;
        }

    coStop
    },10000);

    // Turn off an specific timer
    timer::clear(A);

}