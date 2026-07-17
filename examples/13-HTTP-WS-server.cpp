#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/ws.h>

using namespace nodepp;

void onMain(){

    auto srv = http::server([=]( http_t cli ){

        cli.write_header( 200, header_t({}) );
        cli.write( "hello world!" );

    });

    ws::server( srv );

    srv.onConnect([=]( ws_t cli ){

        cli.onData([=]( string_t data ){
            console::log( ">>", data );
        });

        cli.onClose([=](){
            console::log( "disconnected" );
        }); console::log( "---connected" );

    });

    srv.listen( "localhost", 8000, [=]( socket_t ){
        console::log( ">> http://localhost:8000" );
    });

}