#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/observer.h>

using namespace nodepp;

namespace TEST { namespace OBSERVER {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | observer initialization", [](){
            try {
                observer_t obj ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
             if ( obj.size() != 3 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | observer get", [](){
            try {
                observer_t obj ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
             if ( obj.get("var2").as<int>() != 20 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | observer set", [](){
            try {

                ptr_t<int> val = new int(0);

                observer_t obj ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });

                obj.once( "var2", [=]( any_t a, any_t b ){
                    *val = b.as<int>();
                }); obj.set("var2",30);

             if ( *val != 30 ){ throw 0; }
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