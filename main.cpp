#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/https.h>

using namespace nodepp;

void server() {

    auto ssl = ssl_t();
    auto server = https::server([=]( https_t cli ){ 

        if( cli.method != "POST" || !cli.headers.has( "Content-Length" ) ){ 
            cli.write_header( 404, header_t({}) );
            cli.write( "not allowed" ); return;
        }

        cli.read_body()

        .then([=]( https_t cli ){
            console::log( "->", cli.body );
            cli.write_header( 200, header_t({}) );
            cli.write( "received" );
        })
        
        .fail([=]( except_t err ){
            console::error( err );
        });

    }, &ssl );

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void onMain() {

    worker::add([=](){ server(); process::wait(); return -1; });

}