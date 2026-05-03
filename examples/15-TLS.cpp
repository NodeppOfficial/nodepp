#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/tls.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto ssl    = ssl_t();
    auto server = tls::server( &ssl );

    server.onConnect([=]( ssocket_t cli ){

        console::log("connected" );

        cli.onData([=]( string_t data ){
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

    server.listen( "localhost", 8000, []( ssocket_t ){
        console::log("-> tls://localhost:8000");
    });

}

void client(){

    auto ssl    = ssl_t();
    auto client = tls::client( &ssl );

    client.onConnect([=]( ssocket_t cli ){

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
        console::log("-> tls://localhost:8000");
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