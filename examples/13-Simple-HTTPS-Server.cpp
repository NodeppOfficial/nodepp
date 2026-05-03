#include <nodepp/nodepp.h>
#include <nodepp/https.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain(){

    ssl_t ssl; // ( "./ssl/cert.key", "./ssl/cert.crt" );

    auto server = https::server([=]( https_t cli ){

        console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( date::fulltime() );
        cli.close();

    }, &ssl );

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    /*
    server.listen( "[localhost]", 8000, [=]( ssocket_t server ){
        console::log("server started at https://[::1]:8000");
    });
    */

    server.listen( "localhost", 8000, [=]( ssocket_t server ){
        console::log("server started at https://localhost:8000");
    });

}