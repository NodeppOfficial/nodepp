#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/query.h>

using namespace nodepp;

namespace TEST { namespace QUERY {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | query initialization", [](){
            do { query_t query = nodepp::query::parse( "?var1=10&var2=20&var3=30" );
            if ( query.size() != 3 )   { break; }
            if ( query["var1"]!= "10" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | query searching", [](){
            do {
                query_t query ({
                    { "var1", "10" },
                    { "var2", "20" },
                    { "var3", "30" }
                });
            if ( !query.has("var1") ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | query parsing", [](){
            do {
                string_t out = query::format( query_t({
                    { "var1", "10" },
                    { "var2", "20" },
                    { "var3", "30" }
                }) );
            if( out!="?var1=10&var2=20&var3=30" ){ break; }
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