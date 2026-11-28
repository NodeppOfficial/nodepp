#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/zlib.h>
#include <nodepp/ws.h>

using namespace nodepp;

void onMain(){

    auto srv = http::server([=]( http_t cli ){

        auto file   = file_t( "LICENSE", "r" );
        auto length = string::to_string( file.size() );

        cli.write_header( 200, header_t({
            { "Content-Length"  , length },
            { "Content-Encoding", "gzip" }
        }) );

        zlib::gzip::pipe( file, cli );

    });

    srv.listen( "localhost", 8000, [=]( socket_t ){
        console::log( ">> http://localhost:8000" );
    });

}