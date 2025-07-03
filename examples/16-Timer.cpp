#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    // add: a Coroutine Based Timer
    auto C = timer::add( coroutine::add( COROUTINE(){
        static uchar x = 100;
    coBegin

        while( x-->0 ){
            console::log(" timer task ");
            coNext;
        }

    coFinish
    }) ,1000);

    // Interval: a setInterval equivalent
    auto A = timer::interval([=](){ 
        console::done("set interval");
    },1000);

    // Timeout: a setTimeout equivalent
    auto B = timer::timeout([=](){
        console::error("set timeout");
    },1000);

    // Turn off an specific timer
    // timer::clear(A);

}