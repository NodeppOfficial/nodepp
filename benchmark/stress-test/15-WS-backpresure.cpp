#define NODEPP_EVENT_SCHEDULER NODEPP_SCHEDULER_EPOLL

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/ws.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

atomic_t<int> counter (0);

/*────────────────────────────────────────────────────────────────────────────*/

void client() {

    auto skt = ws::client( "ws://localhost:8000" );
    
    skt.onConnect([=]( ws_t cli ){ 
        console::log( ">> new client", cli.get_fd() );
        cli.close(); counter.sub(1);
    }); /*--------*/ counter.add(1);

    skt.onError([=]( except_t ){ counter.sub(1); });

}

/*────────────────────────────────────────────────────────────────────────────*/

void server(){

    auto server = http::server([=]( http_t cli ){
         cli.write_header( 200, header_t({}) );
         cli.write( "hello world!" );
    });  ws::server( server ); 
    
    server.onConnect([=]( ws_t cli ){
         console::log( ">>", cli.get_fd() );
         cli.onDrain([=](){ cli.free(); });
         stream::pipe( cli );
    });

    server.listen( "0.0.0.0", 8000, [=]( socket_t ){
        console::log("server started at http://localhost:8000");
    });

}

/*────────────────────────────────────────────────────────────────────────────*/

void onMain() { 
    
    if( process::env::get( "MODE" )!="client" ){ server(); } else {
        process::add( coroutine::add( COROUTINE(){
        coBegin do {

            if( counter.get()<10 ){ client(); }

        } while(0); coAgain }) ); 
    }

}

/*────────────────────────────────────────────────────────────────────────────*/
/* BUG FIXED */