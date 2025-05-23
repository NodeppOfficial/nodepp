#include <nodepp/nodepp.h>
#include <nodepp/tls.h>

using namespace nodepp;

ssl_t ssl; // ( "./ssl/cert.key", "./ssl/cert.crt" );

void onMain(){

    auto server = tls::server( &ssl );

    server.onConnect([=]( ssocket_t cli ){

        console::log("connected to:", cli.get_fd());
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

    });

    server.listen( "localhost", 8000, []( ssocket_t srv ){
        console::log("server started at tls://localhost:8000");
    });

}