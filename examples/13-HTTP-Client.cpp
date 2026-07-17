#include <nodepp/nodepp.h>
#include <nodepp/http.h>

using namespace nodepp;

void onMain(){

    fetch_t args;
            args.method  = "GET";
            args.url     = "http://www.google.com/"; // "http://[www.google.com]/" IPv6
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });
    //      args.body    = "Message";                // only for post messages

    http::fetch( args )

    .then([]( http_t cli ){
        console::log( cli.headers["Host"] );
        cli.onData([]( string_t chunk ){
            console::log( chunk );
        }); stream::pipe( cli );
    })

    .fail([]( except_t err ){
        console::error( err );
    });

}