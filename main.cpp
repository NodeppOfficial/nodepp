#include <nodepp/nodepp.h>
#include <nodepp/tls.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto ssl    = ssl_t(); ssl.set_alpn_protocol_list({ "h2", "http/1.1" });

    auto server = tls::server( &ssl );

    server.onConnect([=]( ssocket_t cli ){

        console::log("connected", cli.ssl->get_alpn_protocol() );

        cli.onData([=]( string_t data ){
            cli.write( "<: received" );
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    server.onError([=]( except_t err ){
        console::log( err.what() );
    });

    server.listen( "localhost", 8000, []( ssocket_t ){
        console::log("-> tls://localhost:8000");
    });

}

void onMain() { server(); }