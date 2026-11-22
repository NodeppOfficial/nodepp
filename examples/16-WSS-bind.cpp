#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/https.h>
#include <nodepp/path.h>
#include <nodepp/date.h>
#include <nodepp/wss.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    ssl_t ssl; //ssl_t ssl( "./ssl/cert.key", "./ssl/cert.crt" );

    auto server = https::server([=]( https_t cli ){ 

        if( cli.path == "/path/ws" ){
        if( wss::bind(cli).has_value() ){

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

    server.listen( "localhost", 8000, [=]( ssocket_t server ){
        console::log("server started at https://localhost:8000");
    });

}