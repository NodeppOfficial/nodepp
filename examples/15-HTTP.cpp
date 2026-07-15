#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        file_t file ( "LICENSE", "r" );
        
        cli.write_header( 200, header_t({
            { "Content-Type"   , "text/html" },
            { "Transfer-Encoding", "chunked" },
        //  { "Content-Length", string::to_string( file.size() ) }
        }));
        
        stream::pipe( file, cli );

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

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
        cli.onClose([](){ console::log("closed"); });
        cli.onData ([]( string_t chunk ){
            console::log( chunk );
        }); stream::pipe( cli );
    })

    .fail([]( except_t err ){
        console::error( err );
    });

}

void onMain() {

    worker::add([=](){ server(); process::wait(); return -1; });
    worker::add([=](){ client(); process::wait(); return -1; });

}