#include <nodepp/nodepp.h>
#include <nodepp/http.h>

using namespace nodepp;

void onMain(){

    fetch_t arg;
    arg.method = "GET";
    arg.url    = "http://0.0.0.0:6931/";
    
    ptr_t<promise_t<http_t,except_t>> que ({ http::fetch( arg ) });

    promise::all( que )

    .then([=]( auto que ){

        console::log( que[0].get_value().value().read() );

    })

    .fail([=]( except_t err ){
        console::log( ">>", err );
    });

}