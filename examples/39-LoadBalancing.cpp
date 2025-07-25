#include <nodepp/nodepp.h>

#include <nodepp/cluster.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/date.h>
#include <nodepp/fs.h>

using namespace nodepp;

void server( int process ){

    auto server = http::server([=]( http_t cli ){ 
        
        cli.write_header( 200, header_t({
            { "Content-Type", "text/html" }
        }) );
        
        cli.write("Hello World! \n");
        cli.write("Load Balanced Server");

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("-> http://localhost:8000");
    });

}

void onMain(){

    if( process::is_child() ){ server( os::pid() ); } else {
        for( auto x = os::cpus(); x--; ){
             auto y = cluster::add();
        }
    }

}