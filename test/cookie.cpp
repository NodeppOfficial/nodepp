#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/cookie.h>

using namespace nodepp;

namespace TEST { namespace COOKIE {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | cookie initialization", [](){
            do { cookie_t cookie = nodepp::cookie::parse( "var1=10; var2=20; var3=30" );
            if ( cookie.size()  != 3 )   { break; }
            if ( cookie["var1"] != "10" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | cookie searching", [](){
            do {
                cookie_t cookie ({
                    { "var1", "10" },
                    { "var2", "20" },
                    { "var3", "30" }
                });
             if ( !cookie.has("var1") ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | cookie parsing", [](){
            do {
                string_t out = cookie::format( cookie_t({
                    { "var1", "10" },
                    { "var2", "20" },
                    { "var3", "30" }
                }) );
            if( out!="var1=10; var2=20; var3=30" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        test.onClose.once([=](){
            console::log("\nRESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err) ++; (*totl)++; });
        test.onSkip([=](){ (*skp) ++; (*totl)++; });

        TEST_AWAIT( test );

    }

}}