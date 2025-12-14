#include <nodepp/nodepp.h>
//#include <nodepp/https.h>
#include <nodepp/http.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace HTTP {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | HTTP Fetch (Promise)", [](){
            try { ptr_t<int> x = new int(0);

                fetch_t args;
                        args.url    = "http://www.google.com";
                        args.method = "GET";

                http::fetch( args )

                .then([=]( http_t cli ){
                    if( cli.status==200 ){ *x = 1; }
                   else /*-------------*/{ *x = 2; }
                })

                .fail([=]( except_t ){ *x = -1; });

                while( *x==0 ){ process::next(); }
               switch( *x ){
                    case 1: TEST_DONE(); break;
                    case 2: TEST_FAIL(); break;
                   default: TEST_SKIP(); break;
                }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | HTTP Fetch (await)", [](){
            try { ptr_t<int> x = new int(0);

                fetch_t args;
                        args.url    = "http://www.google.com";
                        args.method = "GET";

                auto fetch = http::fetch( args ).await();

                if( !fetch.has_value() )/*---*/{ TEST_SKIP(); }
                if( fetch.value().status==200 ){ TEST_DONE(); }

                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        /*
        TEST_ADD( test, "TEST 3 | HTTPS Fetch (Promise)", [](){
            try { ptr_t<int> x = new int(0);

                fetch_t args; ssl_t ssl;
                        args.url    = "https://www.google.com";
                        args.method = "GET";

                https::fetch( args, &ssl )

                .then([=]( https_t cli ){
                    if( cli.status==200 ){ *x = 1; }
                   else                  { *x = 2; }
                })

                .fail([=]( except_t ){ *x = -1; });

                while( *x==0 ){ process::next(); }
               switch( *x ){
                    case 1: TEST_DONE(); break;
                    case 2: TEST_FAIL(); break;
                   default: TEST_SKIP(); break;
                }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | HTTPS Fetch (await)", [](){
            try { ptr_t<int> x = new int(0);

                fetch_t args; ssl_t ssl;
                        args.url    = "https://www.google.com";
                        args.method = "GET";

                auto fetch = https::fetch( args, &ssl ).await();

                if( !fetch.has_value() )       { TEST_SKIP(); }
                if( fetch.value().status==200 ){ TEST_DONE(); }

                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });
        */

        test.onClose.once([=](){
            console::log("\nRESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err)++;  (*totl)++; });
        test.onSkip([=](){ (*skp)++;  (*totl)++; });

        TEST_AWAIT( test );

    }

}}