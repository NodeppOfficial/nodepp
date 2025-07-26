#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/query.h>

using namespace nodepp;

namespace TEST { namespace QUERY {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | query initialization", [](){
            try {
                query_t query = nodepp::query::parse( "?var1=10&var2=20&var3=30" );
             if ( query.size() != 3 )/**/{ throw 0; }
             if ( query["var1"] != "10" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | query searching", [](){
            try {
                query_t query ({
                    { "var1", "10" },
                    { "var2", "20" },
                    { "var3", "30" }
                });
             if ( !query.has("var1") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | query parsing", [](){
            try {
                string_t out = query::format( query_t({
                    { "var1", "10" },
                    { "var2", "20" },
                    { "var3", "30" }
                }) );
            if( out!="?var1=10&var2=20&var3=30" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        test.onClose.once([=](){
            console::log("\nRESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err)++;  (*totl)++; });
        test.onSkip([=](){ (*skp)++;  (*totl)++; });

        TEST_AWAIT( test );

    }

}}

// void onMain(){ TEST::CONSOLE::TEST_RUNNER(); }