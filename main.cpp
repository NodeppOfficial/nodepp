#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    timer::interval([=](){
        console::log("AAA");
    },1000);

    timer::interval([=](){
        console::log("BBB");
    },1000);

    timer::interval([=](){
        console::log("CCC");
    },1000);

}