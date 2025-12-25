#include <nodepp/nodepp.h>
#include <nodepp/tls.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto ssl    = ssl_t();
    auto server = tls::server( &ssl );

    server.onConnect([=]( ssocket_t cli ){

        console::log("connected" );

        cli.onData([=]( string_t data ){
            cli.write( "<: received" );
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

void client(){

    auto ssl    = ssl_t();
    auto client = tls::client( &ssl );

    client.onConnect([=]( ssocket_t cli ){

        console::log("connected" );
        auto cin = fs::std_input();
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onDrain.once([=](){
            console::log("closed");
            cin.close();
        });

        stream::pipe( cli );
        stream::pipe( cin );

    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> tls://localhost:8000");
    });

}

void onMain() {

    if( process::env::get("mode")=="client" ) 
      { client(); } else { server(); }

}

// g++ -o main main.cpp -I./include ; ./main ?mode=client