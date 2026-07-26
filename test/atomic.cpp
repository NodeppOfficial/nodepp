#include <nodepp/nodepp.h>
#include <nodepp/atomic.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace ATOMIC {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | atomic -> initialization", [](){
            do { 
                
                atomic_t<int> x( 10 );
                if( x== 10 ){ break; }

                        TEST_FAIL();
            } while(0); TEST_DONE();
        });

        TEST_ADD( test, "TEST 2 | atomic -> add", [](){
            do { 
                
                atomic_t<int> x( 10 ); x += 10;
                if( x== 20 ){ break; }

                        TEST_FAIL();
            } while(0); TEST_DONE();
        });

        TEST_ADD( test, "TEST 3 | atomic -> sub", [](){
            do { 
                
                atomic_t<int> x( 10 ); x -= 5;
                if( x == 5 ){ break; }

                        TEST_FAIL();
            } while(0); TEST_DONE();
        });

        TEST_ADD( test, "TEST 4 | atomic -> swap", [](){
            do {

                atomic_t<int> x( 10 ); 
                auto y = x.swap(20);
                auto z = x.get();

                if( y!= 10 ){ break; }
                if( z!= 20 ){ break; }

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