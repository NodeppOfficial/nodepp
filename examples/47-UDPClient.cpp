#include <nodepp/nodepp.h>
#include <nodepp/udp.h>

using namespace nodepp;

void onMain(){

    auto client = udp::client();
    auto cin    = fs::std_input();

    client.onConnect([=]( socket_t cli ){

        console::log("connected", cli.get_peername() );
    
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
        console::log("-> udp://localhost:8000");
    });

}