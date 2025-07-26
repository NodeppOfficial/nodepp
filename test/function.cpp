#include <nodepp/nodepp.h>
#include <nodepp/function.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace FUNCTION {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | function initialization 1", [](){
            try {
                function_t<int> clb ([=](){ return 1; });
             if ( clb.empty() ){ throw 0; }
             if ( clb() != 1  ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | function initialization 2", [](){
            try {
                function_t<int,int> clb ([=]( int x ){ return x; });
             if ( clb.empty() ){ throw 0; }
             if ( clb(1) != 1 ){ throw 0; }
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