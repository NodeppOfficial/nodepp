#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( socket_t cli ){ 

        cli.write( "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n9\r\nwikipedia\r\n9\r\nwikipedia\r\n9\r\nwikipedia\r\n9\r\nwikipedia\r\n9\r\nwikipedia\r\n" );
        cli.write( "9\r\nwikipedia\r\n9"); 
        cli.write( "\r\nwikipedia\r\n9\r\nwikip");
        cli.write( "edia\r\n9\r\nwikipedia\r\n9\r\nwikipedia\r");
        cli.write( "\n" );

        file_t file ( "LICENSE", "r" );
        stream::pipe( file );

        file.onPipe([=](){ 
            auto size = encoder::hex::atob( file.size() );
            cli.write( size + "\r\n" ); 
        });

        file.onDrain([=](){ cli.write("\r\n"); });

        file.onData([=]( string_t data ){
            cli.write( data );
        });

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