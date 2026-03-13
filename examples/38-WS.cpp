#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/ws.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto server = http::server([=]( http_t cli ){ 

        cli.write_header( 200, header_t({
            { "Content-Security-Policy", "*" }
        }) );

        cli.write("Hello World!");

    }); ws::server( server );

    server.onConnect([=]( ws_t cli ){

        console::log("connected");
        
        cli.onData([=]( string_t data ){
            cli.write( "<: received" );
            console::log( data );
        });

        cli.onClose([=](){ 
            console::log("closed"); 
        });

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void client() {

    auto client = ws::client( "ws://localhost:8000/" );
    
    client.onConnect([=]( ws_t cli ){ 

        auto cin = fs::std_input();
        console::log("connected");

        cli.onData([]( string_t data ){ 
            console::log( data ); 
        });
        
        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onClose([=](){ 
            console::log("closed"); 
            cin.close();
        });

        stream::pipe( cin );

    });

    client.onError([=]( except_t err ){
        console::log( "<>", err.data() );
    });

}

void onMain() {

    if( process::env::get("mode")=="client" ) 
      { client(); } else { server(); }

}

// g++ -o main main.cpp -I./include ; ./main ?mode=client