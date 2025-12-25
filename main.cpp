#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>

using namespace nodepp;

void local_main(){

    limit::set_process_priority( limit::PRIORITY::HIGHEST_PRIORITY );

    auto server = http::server([=]( http_t cli ){ 

    //  console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( "AAA" );

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void onMain() {

    local_main();
    console::log( sizeof( ptr_t<ulong> ) );

}