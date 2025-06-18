#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/wss.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {

    ssl_t ssl; //ssl_t ssl( "./ssl/cert.key", "./ssl/cert.crt" );

    auto client = wss::client( "wss://localhost:8000/", &ssl );
    auto cin    = fs::std_input();
    
    client.onConnect([=]( wss_t cli ){ 
        
        console::log("connected"); 

        cli.onData([]( string_t chunk ){ 
            console::log("client:>",chunk); 
        });
        
        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onClose([](){ 
            console::log("closed"); 
            process::exit(1);
        });

    });

    stream::pipe( cin );

}