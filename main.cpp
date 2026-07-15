#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain(){

    auto server = http::server([=]( socket_t cli ){ 

        auto a = type::cast<http_t>( cli );
        auto b = type::cast<http_t>( cli );

        stream::pipe( a );
        stream::pipe( b );

    });

    server.onError([=]( except_t err ){
        console::log( ">>", err.what() );
    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}