#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        file_t file ( "LICENSE", "r" );

        cli.set_timeout(0);           // disable timeout
    //  file.set_buffer_size( 1024 ); // change stream buffer size
        
        cli.write_header( 200, header_t({
            { "Content-Type"   , "text/html" },
            { "Transfer-Encoding", "chunked" },
        //  { "Content-Length", string::to_string( file.size() ) }
        }));

        cli.onData ([=]( string_t data ){
            console::log( ">>", data );
        }); stream::pipe( cli );
        
        timer ::add ([=](){
            auto msg = regex::format( "hello world! ${0}", process::now() );
            return cli.write( msg ) <= 0 ? -1 : 1 ;
        },1000);

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

        cli.onData ([=]( string_t chunk ){
            cli.write("<: received");
            console::log( chunk );
        }); 
        
        cli.set_timeout(0); // disable timeout
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