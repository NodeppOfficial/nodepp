#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>
#include <nodepp/cluster.h>

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void server( int x ){

    auto server = http::server([=]( http_t cli ){ 

        console::log( cli.path, cli.get_fd() );
        
        cli.write_header( 200, header_t({
            { "content-type", "text/html" }
        }));
        
        cli.write( date::fulltime() );
        cli.close();

    });

    server.listen( "localhost", 8000, [=]( socket_t server ){
        console::log("server started at http://localhost:8000");
    });

}

/*────────────────────────────────────────────────────────────────────────────*/

void spawn( int x ) {

    auto cid = regex::format( "?CPU=${0}", x );
    auto pid = cluster::add( ptr_t<string_t>({ cid }) );

    if( pid.has_value() ){
        pid.value().onClose.once([=](){ spawn(x); });
        pid.value().onData([=]( string_t msg ){
            conio::log( msg );
        });
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){ 

    os::set_process_priority( os::PRIORITY::HIGH_PRIORITY );
    os::set_hard_fileno/*-*/( (uint)-1 );
    os::set_soft_fileno/*-*/( (uint)-1 );

    int y = string::to_uint( process::env::get( "CPU" ) );

    for  ( auto x=os::cpus(); x-->0; ){
    if   ( cluster::is_child() )
         { server(y); break; } 
    else { spawn (x); } }

}

/*────────────────────────────────────────────────────────────────────────────*/