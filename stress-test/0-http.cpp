#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>
#include <nodepp/date.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

inline tcp_t start_http_server(){

    return http::server([=]( http_t cli ){

        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }) );

        if( cli.method == "POST" && cli.headers.has("Content-Length") ){
            ulong len = string::to_ulong( cli.headers["Content-Length"] );
            
            ptr_t<ulong> tmp ( 0UL );
            ptr_t<generator::file::read> _read_ ( 0UL );

            process::add( coroutine::add( COROUTINE(){
            coBegin

                while( !cli.is_closed() && *tmp < len ){
                if   ( (*_read_)( &cli, len - *tmp )==1 ){ coNext; }
                    console::log( "stamp", _read_->data.size(), _read_->data );
                *tmp += _read_->data.size(); }

                cli.write( string::format( "message-size: %d", len ) );
                cli.close();

            coFinish
            }));

        } else {

            cli.write( "hello world" );

        }

    });

}

/*────────────────────────────────────────────────────────────────────────────*/

inline void start_http_client(){

    file_t  file ( "LICENSE", "r" );
    fetch_t arg;
            arg.url    = rand()%2==0 ? "http://localhost:8000" : "http://[localhost]:8000";
            arg.method = rand()%2==0 ? "POST" : "GET";
            arg.headers= header_t({
                { "content-length", string::format( "%lu", file.size() ) }
            });

    http::fetch( arg, nullptr, [=]( http_t cli ){
        if( arg.method == "GET" ){ return; }
        file.onData([=]( string_t data ){
            cli.write( data );
        }); stream::pipe( file );
    })

    .then([=]( http_t cli ){
        console::log( "<<", cli.read() );
    })

    .fail([=]( except_t err ){
        console::error( err.what() );
    });

}

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    promise_t<null_t,except_t>([=]( 
        res_t<null_t> res, rej_t<except_t> rej
    ){

        worker::add([=](){

            tcp_t server = start_http_server();

            server.onError.once([=]( except_t err ){
                rej( err );
            });

            server.listen( "[::0]", 8000, [=]( socket_t ){
                res( nullptr );
            });
            
        process::wait(); return -1; });

    })

    .then([=]( null_t /*unused*/ ){
        console::done( "server started successfuly" );

        worker::add( coroutine::add( COROUTINE(){
        coBegin

            while( true ){
                start_http_client(); 
            process::wait(); }

        coFinish
        }));

    })

    .fail([=]( except_t err ){
        console::error( err.what() );
    });

}

/*────────────────────────────────────────────────────────────────────────────*/