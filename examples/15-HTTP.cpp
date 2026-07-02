#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( "hello world!" );

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

    process::wait();

}

void client() {

    fetch_t args;
            args.method  = "GET";
            args.url     = "http://localhost:8000/";
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });

    http::fetch( args )

    .then([]( http_t cli ){
        console::log( cli.headers["Host"] );
        cli.onData([]( string_t chunk ){
            console::log( chunk );
        }); stream::pipe( cli );
    })

    .fail([]( except_t err ){
        console::error( err );
    });

    process::wait();

}

void onMain() {

    worker::add([=](){ server(); return -1; });
    worker::add([=](){ client(); return -1; });

}