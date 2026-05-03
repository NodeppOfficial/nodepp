#include <nodepp/nodepp.h>
#include <nodepp/tcp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto client = tcp::client();

    client.onOpen([=]( socket_t cli ){

        console::log("connected" );
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose.once([=](){
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
        console::log("-> tcp://localhost:8000");
    });

}