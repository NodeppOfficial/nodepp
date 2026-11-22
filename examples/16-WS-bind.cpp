#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/date.h>
#include <nodepp/ws.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto server = http::server([=]( http_t cli ){ 

        if( cli.path == "/path/ws" ){
        if( ws::bind(cli).has_value() ){

            //[NOTE] be careful, never use write_header() before ws::bind();
            //love this function; reduce CPU cycles
            console::log("connected" );

            cli.onData([=]( string_t data ){
                console::log( data );
            });

            cli.onClose([=](){
                console::log("closed");
            });

        } else {

            cli.write_header( 404, header_t({}) );
            cli.write( "something went wrong!" );

        } return; }

        cli.write_header( 200, header_t({}) );
        cli.write( "Hello World!" );

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}