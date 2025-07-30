#include <nodepp/nodepp.h>
#include <nodepp/tls.h>

using namespace nodepp;

ssl_t ssl; // ( "./ssl/cert.key", "./ssl/cert.crt" );

void onMain(){

    auto server = tls::server( ssl );
    auto cin    = fs::std_input();

    server.onConnect([=]( ssocket_t cli ){

        console::log("connected", cli.get_peername() );

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onClose.once([=](){
            console::log("closed");
        });

        stream::pipe( cli );
        stream::pipe( cin );

    });

    server.listen( "localhost", 8000, []( ssocket_t ){
        console::log("-> tls://localhost:8000");
    });

}
