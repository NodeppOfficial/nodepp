#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( http_t cli ){ 

        cli.write_header( 200, header_t({ 
            { "Transfer-Encoding", "chunked" }
        }));

        cli.onData ([]( string_t chunk ){
            console::log( chunk.size(), "<-", chunk );
        }); 

        timer::add([=](){ if( cli.is_closed() ){ return -1; }
            cli.write( string::format( "hello world from server %llu", process::now() ) );
        return 1; }, 1000 );
        
        cli.set_timeout(0); // < disable timeout
        stream::pipe( cli );

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
                { "Transfer-Encoding", "chunked" }
            });

    http::fetch( args )

    .then([]( http_t cli ){

        cli.onData ([]( string_t chunk ){
            console::log( chunk.size(), "->", chunk );
        }); 

        timer::add([=](){ if( cli.is_closed() ){ return -1; }
            cli.write( string::format( "hello world from client %llu", process::now() ) );
        return 1; }, 2000 );
        
        cli.set_timeout(0); // < disable timeout
        stream::pipe( cli );

    })

    .fail([]( except_t err ){
        console::error( err );
    });

}

void onMain() {

    worker::add([=](){ server(); process::wait(); return -1; });
    worker::add([=](){ client(); process::wait(); return -1; });

}