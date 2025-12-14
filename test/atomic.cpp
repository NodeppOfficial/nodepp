#include <nodepp/nodepp.h>
#include <nodepp/atomic.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace ATOMIC {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | atomic -> initialization", [](){
            try {
                atomic_t<int> x( 10 );
                if( x== 10 ){ TEST_DONE(); }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | atomic -> add", [](){
            try {
                atomic_t<int> x( 10 ); x += 10;
                if( x== 20 ){ TEST_DONE(); }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | atomic -> sub", [](){
            try {
                atomic_t<int> x( 10 ); x -= 5;
                if( x == 5 ){ TEST_DONE(); }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | atomic -> swap", [](){
            try {

                atomic_t<int> x( 10 ); 
                auto y = x.swap(20);
                auto z = x.get();

                if( y!= 10 ){ TEST_FAIL(); }
                if( z!= 20 ){ TEST_FAIL(); }
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