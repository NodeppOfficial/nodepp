#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/date.h>
#include <nodepp/ws.h>
#include <nodepp/fs.h>

using namespace nodepp;

void client( int x ) {

    auto client = ws::client( "ws://localhost:8000/" );

    client.onConnect([=]( ws_t cli ){ for( auto y=100; y-->0; ){ 
        
        auto raw = type::bind( cli );
        auto stt = type::bind( new int(0) );
        auto idx = type::bind( new int(10) );
        auto wrt = type::bind( _file_::write() );

        process::add([=](){ auto _state_ = *stt;
        gnStart while( (*idx)-->0 ){
            coWait( (*wrt)(&raw,regex::format( "Hello World ${0} ${1} ${2}", x, y, *idx ))==1 );
        } gnStop }); 
    
    }});


}

void server(){

    auto server = ws::server();

    server.onConnect([=]( ws_t cli ){

        console::log("connected");

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        cli.onClose([=](){
            console::log("closed");
        });

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

void onMain() { server(); for( auto x=10; x-->0; ){ client(x); } }
