#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    fetch_t args;
            args.method = "GET";
            args.url    = "http://ip-api.com/json/";
            args.headers= header_t({
                { "Host", url::host(args.url) }
            });

    http::fetch( args )

    .then([=]( http_t cli ){

        if( cli.status != 200 ){ 
            console::error( "api not found" );
            return;    
        }

        auto raw = stream::await( cli );
        auto obj = json::parse( raw );

        console::log( "country:", obj["country"].as<string_t>() );
        console::log( "city:"   , obj["city"]   .as<string_t>() );
        console::log( "ip:"     , obj["query"]  .as<string_t>() );

    })

    .fail([=]( except_t err ){
        console::error( err );
    });

}