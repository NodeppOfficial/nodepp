#include <nodepp/nodepp.h>
#include <nodepp/variant.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace VARIANT {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | variant initialization string", [](){
            try { variant_t<string_t,uint,float> mem = "hello world";
             if ( !mem.has_value() )/*---------------*/{ throw 0; }
             if ( mem.as<string_t>() != "hello world" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | variant initialization uint", [](){
            try { variant_t<string_t,uint,float> mem = type::cast<uint>(10);
             if ( !mem.has_value() )    { throw 0; }
             if ( mem.as<uint>() != 10 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | variant initialization float", [](){
            try { variant_t<string_t,uint,float> mem = type::cast<float>(10);
             if ( !mem.has_value() )/*----*/{ throw 0; }
             if ( mem.as<float>() != 10.0f ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | variant error handling", [](){
            try { variant_t<string_t,uint,float> mem = object_t({ { "var1", 10 } });
             if ( !mem.has_value() )/*----------------------*/{ throw 0; }
             if ( mem.as<object_t>()["var1"].as<int>() != 10 ){ throw 0; }
                              TEST_FAIL();
            } catch ( ... ) { TEST_DONE(); }
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