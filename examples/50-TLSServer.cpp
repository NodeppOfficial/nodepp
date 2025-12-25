#include <nodepp/nodepp.h>
#include <nodepp/tls.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto ssl    = ssl_t();
    auto server = tls::server( &ssl );

    server.onConnect([=]( ssocket_t cli ){

        console::log("connected" );

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onDrain.once([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    server.listen( "localhost", 8000, []( ssocket_t ){
        console::log("-> tls://localhost:8000");
    });

}
