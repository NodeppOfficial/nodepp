#include <nodepp/nodepp.h>
#include <nodepp/cluster.h>
#include <nodepp/popen.h>

using namespace nodepp;

void onMain() {

    if( process::is_parent() ){

        auto pid = cluster::add();

    if( !pid.has_value() ){ throw except_t( "something went wrong" ); }

        pid.value().onDrain([=](){ console::log("Done"); });
        pid.value().onData ([=]( string_t data ){
            console::log("->",data);
        });
    
    } else {
        
        process::add( coroutine::add( COROUTINE(){
            static uint x = 10; 
        coBegin
            while( x-->0 ){
                console::log("Hello World!");
                coDelay(1000);
            }
        coFinish
        }));

    }

}