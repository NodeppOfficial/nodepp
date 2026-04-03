#include <nodepp/nodepp.h>
#include <nodepp/udp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto client = udp::client();

    client.onConnect([=]( socket_t cli ){

        console::log("connected" );
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    client.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    /*
    client.connect( "[localhost]", 8000, []( socket_t srv ){
        console::log("-> tcp://[::1]:8000");
    });
    */

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> udp://localhost:8000");
    });

}