#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/date.h>

using namespace nodepp;

void server(){

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( date::fulltime() );
        cli.close();

    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void onMain(){

    auto krn = process::kernel();

    worker::add([=](){
        
        server();

        krn.loop_add( coroutine::add( COROUTINE(){
        coBegin

            while( true ){
                console::log( "hello world" );
            coDelay(1000); }

        coFinish
        }) );

        krn.wake();

    process::wait(); return -1; });

}