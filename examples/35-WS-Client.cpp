#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/fs.h>
#include <nodepp/ws.h>

using namespace nodepp;

void onMain() {

    auto client = ws::client( "ws://localhost:8000/" );
    auto cin    = fs::std_input();
    
    client.onConnect([=]( ws_t cli ){ 
        
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