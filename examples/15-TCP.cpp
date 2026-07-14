#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/tcp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto server = tcp::server();

    server.onConnect([=]( socket_t cli ){

        console::log("connected" );
    
        cli.onData([=]( string_t data ){
            console::log( "server read>>", data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );
        timer ::add ([=](){
            auto msg = regex::format( "hello world! ${0}", process::now() );
            return cli.write( msg ) <= 0 ? -1 : 1 ;
        },1000);


    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    server.listen( "localhost", 8000, []( socket_t srv ){
        console::log("-> tcp://localhost:8000");
    });

}

void client(){

    auto client = tcp::client();

    client.onOpen([=]( socket_t cli ){

        console::log("connected" );
    
        cli.onData([=]( string_t data ){
            console::log( "client read>>", data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

        stream::pipe( cli );
        timer ::add ([=](){
            auto msg = regex::format( "hello world! ${0}", process::now() );
            return cli.write( msg ) <= 0 ? -1 : 1 ;
        },1000);

    });

    client.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    client.connect( "localhost", 8000, []( socket_t cli ){
        console::log("-> tcp://localhost:8000");
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