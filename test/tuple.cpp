#include <nodepp/nodepp.h>
#include <nodepp/tuple.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace TUPLE {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | tuple initialization", [](){
            do { tuple_t<int,float,string_t> tp ( 10, 10.50, "hello world!" );
            if ( tuple::get<0>(tp) != 10 )/*--------*/{ break; }
            if ( tuple::get<1>(tp) != 10.50 )/*-----*/{ break; }
            if ( tuple::get<2>(tp) != "hello world!" ){ break; }
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