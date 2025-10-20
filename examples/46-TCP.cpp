#include <nodepp/nodepp.h>
#include <nodepp/tcp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto server = tcp::server();

    server.onConnect([=]( socket_t cli ){

        console::log("connected" );
        auto cin = fs::std_input();

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

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("-> tcp://localhost:8000");
    });

}

void client(){

    auto client = tcp::client();

    client.onOpen([=]( socket_t cli ){

        console::log("connected" );
        auto cin = fs::std_input();
    
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

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> tcp://localhost:8000");
    });

}

void onMain() {

    if( process::env::get("mode")=="client" ) 
      { client(); } else { server(); }

}

// g++ -o main main.cpp -I./include ; ./main ?mode=client