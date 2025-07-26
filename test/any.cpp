#include <nodepp/nodepp.h>
#include <nodepp/object.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace ANY {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | any initialization string", [](){
            try { any_t mem = "hello world";
             if ( !mem.has_value() )/*---------------*/{ throw 0; }
             if ( mem.as<string_t>() != "hello world" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | any initialization uint", [](){
            try { any_t mem = type::cast<uint>(10);
             if ( !mem.has_value() )    { throw 0; }
             if ( mem.as<uint>() != 10 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | any initialization float", [](){
            try { any_t mem = type::cast<float>(10);
             if ( !mem.has_value() )/*----*/{ throw 0; }
             if ( mem.as<float>() != 10.0f ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | any initialization object", [](){
            try { any_t mem = object_t({ { "var1", 10 } });
             if ( !mem.has_value() )/*----------------------*/{ throw 0; }
             if ( mem.as<object_t>()["var1"].as<int>() != 10 ){ throw 0; }
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