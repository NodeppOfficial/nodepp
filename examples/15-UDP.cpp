#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/udp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto server = udp::server();

    server.onConnect([=]( socket_t cli ){

        console::log("connected" );

        cli.onData([=]( string_t data ){
            auto tmp = cli.get_client_address();
            /*async logic or coroutines*/
            cli.set_client_address( tmp );
            cli.write( "<: received" );
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );

    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("-> udb://localhost:8000");
    });

}

void client(){

    auto client = udp::client();

    client.onConnect([=]( socket_t cli ){

        console::log("connected" );
        auto cin = fs::std_input();
    
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onClose([=](){
            console::log("closed");
            cin.close();
        });

        stream::pipe( cli );
        stream::pipe( cin );

    });

    client.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> udp://localhost:8000");
    });

}

void onMain() {

    worker::add( coroutine::add( COROUTINE(){
    coBegin /*--*/ ; server();
    process::wait(); coFinish }));

    worker::add( coroutine::add( COROUTINE(){
    coBegin /*--*/ ; client();
    process::wait(); coFinish }));

}