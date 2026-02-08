#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/fs.h>
#include <nodepp/ws.h>

using namespace nodepp;

void onMain() {

    auto client = ws::client( "ws://localhost:8000/" );
    
    client.onConnect([=]( ws_t cli ){ 

        console::log("connected", cli.get_peername() );

        cli.onData([]( string_t data ){ 
            console::log( data ); 
        });
        
        cli.onClose([=](){ 
            console::log("closed"); 
        });

    });

    client.onError([=]( except_t err ){
        console::log( "<>", err.data() );
    });

}