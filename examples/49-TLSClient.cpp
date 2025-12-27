#include <nodepp/nodepp.h>
#include <nodepp/tls.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto ssl    = ssl_t();
    auto client = tls::client( &ssl );

    client.onConnect([=]( ssocket_t cli ){

        console::log("connected" );
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onDrain.once([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> tls://localhost:8000");
    });

}
