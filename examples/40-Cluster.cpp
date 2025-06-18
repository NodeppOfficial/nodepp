#include <nodepp/nodepp.h>
#include <nodepp/cluster.h>
#include <nodepp/popen.h>

using namespace nodepp;

void onMain() {

    auto pid = cluster::add();

    if( process::is_parent() ){
        pid.onDrain([=](){ console::log("Done"); });
        pid.onData([=]( string_t data ){
            console::log("->",data);
        });
    } else {
        process::add([=](){
            static uint x = 10; 
        coStart
            while( x-->0 ){
                pid.write( string::format( "hello world %d", x ) );
                coDelay(1000);
            }
        coStop
        });
    }

}