#include <nodepp/nodepp.h>
#include <nodepp/observer.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace OBSERVER {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | observer initialization", [](){
            observer_t obj ({
                { "var1", 10 },
                { "var2", 20 },
                { "var3", 30 },
            });
            if ( obj.size() != 3 )
               { TEST_FAIL(); } TEST_DONE();
        });

        TEST_ADD( test, "TEST 2 | observer get", [](){
            observer_t obj ({
                { "var1", 10 },
                { "var2", 20 },
                { "var3", 30 },
            });
            if ( obj.get("var2").as<int>() != 20 )
               { TEST_FAIL(); } TEST_DONE();
        });

        TEST_ADD( test, "TEST 2 | observer set", [](){

            ptr_t<int> val ( 0UL );

            observer_t obj ({
                { "var1", 10 },
                { "var2", 20 },
                { "var3", 30 },
            });

            obj.once( "var2", [=]( ptr_t<observer_t> self, any_t, any_t b ){
                *val = b.as<int>();
            }); obj.set("var2",30);

            if ( *val != 30 )
               { TEST_FAIL(); } TEST_DONE();
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