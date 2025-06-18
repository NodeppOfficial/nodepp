#include <nodepp/nodepp.h>
#include <nodepp/tcp.h>

using namespace nodepp;

void onMain(){

    auto client = tcp::client();

    client.onOpen([=]( socket_t cli ){
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose.once([=](){
            console::log("closed");
        });

        cli.write("Hello World!");

    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("client connected to tcp://localhost:8000");
    });

}