#include <nodepp/nodepp.h>
#include <nodepp/https.h>

using namespace nodepp;

void onMain(){

    ssl_t ssl; // ( "./ssl/cert.key", "./ssl/cert.crt" );

    fetch_t args;
            args.method = "GET";
            args.url = "https://www.mamp.one/wp-content/uploads/2024/09/image-resources2.jpg";
            args.headers = header_t({
                { "Host", url::host(args.url) }
            });
            
        //  args.file = file_t("PATH","r");
        //  args.body = "MYBODY";

    https::fetch( args, &ssl )

    .then([]( https_t cli ){

        cli.onDrain([](){ console::log( "done" ); });
        cli.onData([=]( string_t data ){
            console::log( "->", data.size() );
        });

        if( cli.status != 200 ){ 
            console::log( "image not found" );
            return;
        }

        file_t file ( "image.jpg", "w" );
        stream::pipe( cli, file );
        
    })

    .fail([]( except_t err ){
        console::error( err );
    });

}