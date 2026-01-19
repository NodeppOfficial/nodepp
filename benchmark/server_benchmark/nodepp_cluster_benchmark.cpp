#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void worker_main(){ worker::add([](){

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

process::wait(); return -1; }); }

void onMain() { for( auto x=os::cpus(); x-->0; ){

    worker_main(); 

}}