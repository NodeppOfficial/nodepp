#include <nodepp/nodepp.h>
#include <nodepp/udp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto server = udp::server();

    server.onConnect([=]( socket_t cli ){

        console::log("connected" );

        cli.onData([=]( string_t data ){
            cli.write( "<: received" );
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("-> udb://localhost:8000");
    });

}

void client(){

    auto client = udp::client();

    client.onConnect([=]( socket_t cli ){

        console::log("connected" );
        auto cin = fs::std_input();
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onClose([=](){
            console::log("closed");
            cin.close();
        });

        stream::pipe( cli );
        stream::pipe( cin );

    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> udp://localhost:8000");
    });

}

void onMain() {

    if( process::env::get("mode")=="client" ) 
      { client(); } else { server(); }

}

// g++ -o main main.cpp -I./include ; ./main ?mode=client