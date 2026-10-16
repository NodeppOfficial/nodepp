#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain(){

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "Content-Type"     , "text/html" },
            { "Transfer-Encoding", "chunked"   },
        //  { "Content-Length", string::to_string( file.size() ) }
        }));

        if( cli.path != "/" ){ cli.close(); return; }

        cli.write( R"(
            <!DOCTYPE html> <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Document</title>
            </head>
            <body style="transform: scaleY(-1);">
        )" );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            while( true ){

                cli.write( regex::format( NODEPP_STRINGIFY(
                    <h1 style="transform: scaleY(-1);"> 
                        Hello World! - ${0} 
                    </h1>
                ) , process::now() ) );

            coDelay(1000); }

        coFinish
        }));

    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    /*
    server.listen( "[localhost]", 8000, [=]( socket_t server ){
        console::log("server started at http://[::1]:8000");
    });
    */

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}