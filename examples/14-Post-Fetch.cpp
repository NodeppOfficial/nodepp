#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( http_t cli ){ 

        if( cli.method != "POST" || !cli.headers.has( "Content-Length" ) ){ 
            cli.write_header( 404, header_t({}) );
            cli.write( "not allowed" ); return;
        }

        cli.read_body()

        .then([=]( http_t cli ){
            console::log( "->", cli.body );
            cli.write_header( 200, header_t({}) );
            cli.write( "received" );
        })
        
        .fail([=]( except_t err ){
            console::error( err );
        });

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void client() {

    fetch_t args;
            args.body    = "hello world!";
            args.method  = "POST";
            args.url     = "http://localhost:8000/";
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });

    http::fetch( args )

    .then([]( http_t cli ){
        console::log( cli.read() );
    })

    .fail([]( except_t err ){
        console::error( err );
    });

}

void onMain() {

    worker::add([=](){ server(); process::wait(); return -1; });
    worker::add([=](){ client(); process::wait(); return -1; });

}