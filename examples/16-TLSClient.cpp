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

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    client.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    /*
    client.connect( "[localhost]", 8000, []( ssocket_t srv ){
        console::log("-> tcp://[::1]:8000");
    });
    */

    client.connect( "localhost", 8000, []( ssocket_t cli ){
        console::log("-> tls://localhost:8000");
    });

}
