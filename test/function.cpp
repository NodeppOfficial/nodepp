#include <nodepp/nodepp.h>
#include <nodepp/function.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace FUNCTION {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | function initialization 1", [](){
            do { function_t<int> clb ([=](){ return 1; });
            if ( clb.empty() ){ break; }
            if ( clb() != 1  ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | function initialization 2", [](){
            do {
                function_t<int,int> clb ([=]( int x ){ return x; });
             if ( clb.empty() ){ break; }
             if ( clb(1) != 1 ){ break; }
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