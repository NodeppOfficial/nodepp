#include <nodepp/nodepp.h>
#include <nodepp/tuple.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace TUPLE {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | tuple initialization", [](){
            try { tuple_t<int,float,string_t> tp ( 10, 10.50, "hello world!" );
             if ( tuple::get<0>(tp) != 10 )/*--------*/{ throw 0; }
             if ( tuple::get<1>(tp) != 10.50 )/*-----*/{ throw 0; }
             if ( tuple::get<2>(tp) != "hello world!" ){ throw 0; }
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