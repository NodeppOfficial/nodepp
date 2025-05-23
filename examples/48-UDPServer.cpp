#include <nodepp/nodepp.h>
#include <nodepp/udp.h>

using namespace nodepp;

void onMain(){

    auto server = udp::server();

    server.onConnect([=]( socket_t cli ){
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

    });

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("server started at udp://localhost:8000");
    });

}