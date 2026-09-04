#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/fs.h>
#include <nodepp/ws.h>

using namespace nodepp;

void onMain() {

    auto client = ws::client( "ws://localhost:8000/" ); // "ws://[localhost]:8000" IPv6
    auto cin    = fs::std_input();
    
    client.onConnect([=]( ws_t cli ){ 

        console::log("connected" );

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onData([]( string_t data ){ 
            console::log( data ); 
        });
        
        cli.onDrain.once([=](){ 
            console::log("closed"); 
            cin.close();
        });

        stream::pipe( cin );

    });

    client.onError([=]( except_t err ){
        console::log( "<>", err.data() );
    });

}