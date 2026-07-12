#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>
#include <nodepp/os.h>

using namespace nodepp;

void onParallel(){

    auto server = http::server([=]( http_t cli ){ 

    //  console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( "<h1>Hello, World!</h1>" );

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void onMain(){
worker::parallel( &onParallel, os::cpus() ); }