#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/ws.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server(){

    auto server = http::server([=]( http_t cli ){ 

        cli.write_header( 200, header_t({
            { "Content-Security-Policy", "*" }
        }) );

        cli.write("Hello World!");

    }); ws::server( server );

    server.onConnect([=]( ws_t cli ){

        console::log("connected");
        
        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose([=](){ 
            console::log("closed"); 
        });

        timer::add([=](){
               cli.write( string::format( "server %lu", process::now() ) );
        return cli.is_closed() ? -1 : 1; }, 10 );

    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void client() {

    auto client = ws::client( "ws://localhost:8000/" );
    
    client.onConnect([=]( ws_t cli ){ 

        cli.write( string::format( "client %lu", process::now() ) );
        cli.close();

    });

    client.onError([=]( except_t err ){
        console::log( "<>", err.data() );
    });

}

void onMain() {

    worker::add( coroutine::add( COROUTINE(){
    coBegin /*--*/ ; server();
    process::wait(); coFinish }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){

            worker::add( coroutine::add( COROUTINE(){
            coBegin /*--*/ ; client();
            process::wait(); coFinish }));

        coDelay( 10 ); }

    coFinish
    }));

}