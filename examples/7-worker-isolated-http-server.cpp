#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/invoke.h>
#include <nodepp/http.h>
#include <nodepp/os.h>

using namespace nodepp;

mutex_t  mutx;

void isolated_event_loop( uint cpu_id ){ worker::add([=](){ 
    //  this worker runs it's own event loop in parallel

    auto server = http::server([=]( http_t cli ){

        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }) );

        cli.write( _STRING_(
            <h1> Hello world! </h1>
            <h3> load balanced nodepp server </h3>
        ));

    });

    server.listen( "localhost", 8000, [=]( socket_t ){
        console::log( "-> http://localhost:8000" );
    } );
    
    process::wait();

return -1; }); }

void onMain(){

    for( auto x=os::cpus(); x-->0; ){
         isolated_event_loop( x );
    }

}