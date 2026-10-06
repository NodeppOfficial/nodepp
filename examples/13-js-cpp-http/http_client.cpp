#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    fetch_t args;
            args.method  = "GET";
            args.url     = "http://localhost:8000/"; // "http://[www.google.com]/" IPv6
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });

        //  args.file = file_t("PATH","r");
        //  args.body = "MYBODY";

    http::fetch( args )

    .then([]( http_t cli ){

        auto body = cli.read_body().await().value();
        console::log( ">>", body );

    })

    .fail([]( except_t err ){
        console::error( err );
    });

}