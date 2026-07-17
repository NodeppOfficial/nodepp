#include <nodepp/nodepp.h>
#include <nodepp/https.h>
#include <nodepp/wss.h>

using namespace nodepp;

void onMain(){

    auto srv = https::server([=]( https_t cli ){

        cli.write_header( 200, header_t({}) );
        cli.write( "hello world!" );

    });

    wss::server( srv );

    srv.onConnect([=]( wss_t cli ){

        cli.onData([=]( string_t data ){
            console::log( ">>", data );
        });

        cli.onClose([=](){
            console::log( "disconnected" );
        }); console::log( "---connected" );

    });

}