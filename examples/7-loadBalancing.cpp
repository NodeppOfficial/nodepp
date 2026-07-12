#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/date.h>
#include <nodepp/os.h>

using namespace nodepp;
atomic_t<int> cpu_id=0;

void onParallel(){

    os::pin_worker_to_cpu( cpu_id.add(1) );

    auto server = http::server([=]( http_t cli ){ 

        string_t msg = "hello world!";
        
        cli.write_header( 200, header_t({
            { "content-length", string::to_string(msg.size()) },
            { "content-type", "text/html" }
        }));
        
        cli.write( regex::format( "${0}\r\n", msg ) );

    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void onMain(){ worker::parallel( &onParallel, os::cpus() ); }