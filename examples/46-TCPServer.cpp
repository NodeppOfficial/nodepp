#include <nodepp/nodepp.h>
#include <nodepp/tcp.h>

using namespace nodepp;

void onMain(){

    auto server = tcp::server();
    auto cin    = fs::std_input();

    server.onConnect([=]( socket_t cli ){

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

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("-> tcp://localhost:8000");
    });

}
