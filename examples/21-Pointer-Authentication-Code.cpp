#define NODEPP_ALLOW_THROW_EXCEPTION 0

#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/json.h>

using namespace nodepp;

void server() {

    auto server = http::server([=]( http_t cli ){ 

        cli.set_timeout(0); // disable timeout

        cli.write_header( 200, header_t({
            { "content-type", "text/plain" }
        }) );

        auto msk = (uchar_64) &cli;

        cli.onData([=]( string_t data ){

            auto body = json::parse( data );
            auto ofc  = string::to_u64( body["pac"].as<string_t>() );
            auto pac  = encoder::pac::btoa( ofc, msk );

            if( pac ){
                console::done( "this address was signed by me:", pac );
            } else {
                console::error( "this is an evil address", pac );
            }

        });

        stream::pipe( cli ); timer::add([=](){

            auto val = process::now(); // or any pointer as (uchar_64)
            auto pac = encoder::pac::atob( val, msk );

            return cli.write( json::format( object_t({
                { "pac", string::to_string( pac ) }
            }) ))==0 ? -1 : 1;

        },1000);
        
    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void client() {

    fetch_t args;
            args.method  = "GET";
            args.url     = "http://localhost:8000/";

            args.headers = header_t({
                { "Host", url::host(args.url) }
            });

    http::fetch( args )

    .then([=]( http_t cli ){

        cli.set_timeout(0); // disable timeout
        
        cli.onData([=]( string_t data ){

            auto body = json::parse( data );
            auto rdom = os::rand() ^ ( os::rand() << 32 );
            auto pac  = string::to_u64( body["pac"].as<string_t>() );

            body["pac"] = string::to_string( rdom%2==0 ? rdom : pac );
            cli.write( json::format( body ) );

        });

        stream::pipe( cli );

    })

    .fail([=]( except_t err ){ console::error( err ); });

}

void onMain() {

    worker::add([=](){ server(); process::wait(); return -1; });
    worker::add([=](){ client(); process::wait(); return -1; });

}