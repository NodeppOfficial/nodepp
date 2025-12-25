#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/https.h>
#include <nodepp/path.h>
#include <nodepp/wss.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    ssl_t ssl; //ssl_t ssl( "./ssl/cert.key", "./ssl/cert.crt" );

    auto server = https::server([=]( https_t cli ){ 

        cli.write_header( 200, header_t({
            { "Content-Security-Policy", "*" }
        }) );

        cli.write("Hello World!");

    }, &ssl ); wss::server( server );

    server.onConnect([=]( wss_t cli ){

        console::log("connected");
        
        cli.onData([=]( string_t data ){
            cli.write( "<: received" );
            console::log( data );
        });

        cli.onDrain([=](){ 
            console::log("closed"); 
        });

    });

    server.listen( "localhost", 8000, [=]( ssocket_t server ){
        console::log("server started at https://localhost:8000");
    });

}

void client() {

    ssl_t ssl; //ssl_t ssl( "./ssl/cert.key", "./ssl/cert.crt" );

    auto client = wss::client( "wss://localhost:8000/", &ssl );
    
    client.onConnect([=]( wss_t cli ){ 
        
        auto cin = fs::std_input();
        console::log("connected"); 

        cli.onData([]( string_t data ){ 
            console::log( data ); 
        });
        
        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onDrain([=](){ 
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