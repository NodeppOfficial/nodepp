#include <nodepp/nodepp.h>
#include <nodepp/udp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto server = udp::server();

    server.onConnect([=]( socket_t cli ){

        console::log("connected", cli.get_peername() );
        auto cin = fs::std_input();

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onDrain.once([=](){
            console::log("closed");
            cin.close();
        });

        stream::pipe( cli );
        stream::pipe( cin );

    });

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("-> udp://localhost:8000");
    });

}
