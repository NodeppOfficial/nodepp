#include <nodepp/nodepp.h>
#include <nodepp/https.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    ssl_t ssl; // ( "./ssl/cert.key", "./ssl/cert.crt" );

    fetch_t args;
            args.method = "GET";
            args.url = "https://www.mamp.one/wp-content/uploads/2024/09/image-resources2.jpg";
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });
            
        //  args.body = "MYBODY";

    https::fetch( args, &ssl )

    .then([]( https_t cli ){

        auto b64 = crypto::encoder::BASE64();
        auto out = fs::writable( "index.html" );
        
        cli.onData ([=]( string_t data ){ b64.update(data); });
        b64.onData ([=]( string_t data ){ out.write (data); });
        b64.onClose([=](){ out.write( "'>" ); });

        out.write( "<img src='data:image/png;base64, " );
        stream::pipe( cli );

    })

    .fail([]( except_t err ){
        console::error( err );
    });

}