#include <nodepp/nodepp.h>
#include <nodepp/cluster.h>
#include <nodepp/popen.h>

using namespace nodepp;

void onMain() {

    if( process::is_parent() ){
        auto pid = cluster::add();

        pid.onDrain([=](){ console::log("Done"); });
        pid.onData([=]( string_t data ){
            console::log("->",data);
        });
    
    } else {
        process::add([=](){
            static uint x = 10; 
        coStart
            while( x-->0 ){
                console::log("Hello World!");
                coDelay(1000);
            }
        coStop
        });
    }

}