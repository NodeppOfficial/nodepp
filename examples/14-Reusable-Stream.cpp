#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto server = http::server([=]( http_t cli ){ 

        cli.write_header( 200, header_t({}) );
        cli.set_reusable( true );

        ptr_t<http_t> self = type::bind( cli );
        ptr_t<int> x ( 0UL, 0x00 );

        function_t<int> clb ([=](){ if( *x<3 ){
            cli.write( "\n----\n" );
            stream::pipe( fs::readable("LICENSE"), *self );
            (*x)++; /**/ return self->is_reusable() ? 1:-1; 
        } self->free();  return -1; });

        cli.onDrain([=](){ if( clb()==-1 ){ clb.free(); } });
        cli.onClose([=](){
            console::log( "DONE" );
        });

        cli.close();

    });

    server.listen( "[::0]", 8000, [=]( socket_t ){
        console::log( "-> http://localhost:8000/" );
    });

}