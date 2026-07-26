#include <nodepp/nodepp.h>
#include <nodepp/object.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace ANY {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | any initialization string", [](){
            do { any_t mem = "hello world";
            if ( !mem.has_value() )/*---------------*/{ break; }
            if ( mem.as<string_t>() != "hello world" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | any initialization uint", [](){
            do { any_t mem = type::cast<uint>(10);
            if ( !mem.has_value() )    { break; }
            if ( mem.as<uint>() != 10 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | any initialization float", [](){
            do { any_t mem = type::cast<float>(10);
            if ( !mem.has_value() )/*----*/{ break; }
            if ( mem.as<float>() != 10.0f ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | any initialization object", [](){
            do { any_t mem = object_t({ { "var1", 10 } });
            if ( !mem.has_value() )/*----------------------*/{ break; }
            if ( mem.as<object_t>()["var1"].as<int>() != 10 ){ break; }
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