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

        cli.write_header( 200, header_t({
            { "Content-Security-Policy", "*" }
        }) );

        cli.write("Hello World!");

    }, &ssl ); wss::server( server );

    server.onConnect([=]( wss_t cli ){

        console::log("connected");

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onDrain([=](){
            console::log("closed");
        });

    });

    server.listen( "localhost", 8000, [=]( ssocket_t server ){
        console::log("server started at https://localhost:8000");
    });

}