#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/http.h>
#include <nodepp/ws.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace WS { worker_t SERVER() {
return worker::add([=](){

    auto server = http::server([=]( http_t cli ){
        thread_local static int x = 0;
        cli.write_header( 200, header_t({}) );
        cli.write( string::to_string(x) ); x++;
    });

    ws::server( server );

    server.onConnect([=]( ws_t cli ){
        thread_local static int x = 0;
        cli.write( string::to_string(x) );
    x++; });

    server.listen( "[::0]", 8000 );
    process::wait();

return -1; });
}}}

namespace TEST { namespace WS {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE(); 
        auto srv  = SERVER();

        TEST_ADD( test, "TEST 1 | WS Client", [](){
            do{ ptr_t<int> x ( 0UL, (int)0x00 )
                
                auto cli = ws::client( "ws://localhost:8000" );
                cli.onError  ([=]( except_t      ){ (*x)--; });
                cli.onConnect([=]( ws_t     cli  ){
                cli.onData   ([=]( string_t data ){ 
                    (*x)++; cli.close();
                }); });

                while ( *x==0 ){ process::next(); }
                switch( *x ){
                   case -1: TEST_DONE(); break;
                   case  1: TEST_FAIL(); break;
                   default: TEST_SKIP(); break;
                }

                        TEST_FAIL();
            } while(0); TEST_FAIL();
        });
        
        TEST_ADD( test, "TEST 2 | HTTP Fetch", [](){
            do{ ptr_t<int> x ( 0UL, (int)0x00 );

                fetch_t args;
                        args.url    = "http://localhost:8000";
                        args.method = "GET";

                agent_t agent;
                        agent.conn_timeout = 5000;

                http::fetch( args, &agent )

                .then([=]( http_t cli ){
                    if( cli.status==200 ){ *x = 1; }
                    else /*------------*/{ *x = 2; }
                })

                .fail([=]( except_t ){ *x = -1; });

                while ( *x==0 ){ process::next(); }
                switch( *x    ){
                    case 1 : TEST_DONE(); break;
                    case 2 : TEST_FAIL(); break;
                    default: TEST_SKIP(); break;
                }

                        TEST_FAIL();
            } while(0); TEST_FAIL();
        });

        test.onClose.once([=](){
            console::log("\nRESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err) ++; (*totl)++; });
        test.onSkip([=](){ (*skp) ++; (*totl)++; });

        TEST_AWAIT( test ); srv.close();

    }

}}