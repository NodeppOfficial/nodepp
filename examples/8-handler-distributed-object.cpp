#include <nodepp/nodepp.h>
#include <nodepp/tcp.h>
#include <nodepp/json.h>
#include <nodepp/timer.h>
#include <nodepp/worker.h>
#include <nodepp/handler.h>

using namespace nodepp;

void server(){

    handler_t<string_t> redux;

    auto srv = tcp_t([=]( socket_t cli ){

        auto hdl = redux.create();
        cli.write( string::to_string(hdl) );

        cli.onData([=]( string_t data ){
            auto obj = json::parse( data ); if( ! obj.has_value() ){ return; }
            auto addr= string::to_u64( obj["addr"].as<string_t>() );
            int    c = redux.update  ( addr, obj["data"].as<string_t>() );
            console::log( ">>", c, addr );
        });

        cli.onClose([=](){
            console::log( "disconnected server" );
        });

        timer::add([=](){
            if( cli.is_closed() ){ return -1; }
            auto val = redux.read( hdl );
            if( val.null() )/*-*/{ return  1; }
            console::log( "->", *val );
        return 1; },1000);

        stream::pipe( cli );

    });

    srv.listen( "[::0]", 3000, [=]( socket_t cli ){
        console::log( "tcp://localhost:3000 server" );
    });

}

void client(){

    auto srv = tcp_t([=]( socket_t cli ){
    auto adr = ptr_t<uchar_64>( 0UL );
 
        timer::add([=](){
            
            if( cli.is_closed() ){ return -1; }

            cli.write( json::stringify( object_t({
                { "data", regex::format( "hello world ${0}", rand()%1000 ) },
                { "addr", string::to_string( *adr ) }
            }) ));

        return 1; },1000);
        
        cli.onData ([=]( string_t data ){ *adr = string::to_u64( data ); });
        cli.onClose([=](){
            console::log( "disconnected client" );
        }); console::log( "connected client", cli.get_fd() );

        stream::pipe( cli );

    });

    srv.connect( "[::0]", 3000, [=]( socket_t cli ){
        console::log( "tcp://localhost:3000 client" );
    });

}

void onMain(){

    worker::add( coroutine::add( COROUTINE(){
    coBegin /*--*/ ; server();
    process::wait(); coFinish }));

    worker::add( coroutine::add( COROUTINE(){
    coBegin /*--*/ ; client();
    process::wait(); coFinish }));

}