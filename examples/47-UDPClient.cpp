#include <nodepp/nodepp.h>
#include <nodepp/udp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto client = udp::client();

    client.onConnect([=]( socket_t cli ){

        console::log("connected", cli.get_peername() );
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> udp://localhost:8000");
    });

}