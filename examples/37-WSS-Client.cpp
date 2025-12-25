#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/wss.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {

    ssl_t ssl; //ssl_t ssl( "./ssl/cert.key", "./ssl/cert.crt" );

    auto client = wss::client( "wss://localhost:8000/", &ssl );
    
    client.onConnect([=]( wss_t cli ){ 
        
        console::log("connected"); 

        cli.onData([]( string_t data ){ 
            console::log( data ); 
        });
        
        cli.onDrain([=](){ 
            console::log("closed"); 
        });

    });

    client.onError([=]( except_t err ){
        console::log( "<>", err.data() );
    });

}