#include <nodepp/nodepp.h>
#include <nodepp/loop.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace LOOP {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | EVloop initialization", [](){
            do { loop_t ev; ptr_t<int> x ( 0UL );
                 ev.add([=](){ *x = 10; return -1; }); 
                 ev.next();
            if ( *x != 10 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 1 | EVloop iteration 1", [](){
            do { loop_t ev; ptr_t<int> x ( 0UL );
                 ev.add([=](){ *x += 10; return 1; }); 
                 ev.next(); ev.next(); ev.next();
            if ( *x != 30 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 1 | EVloop iteration 2", [](){
            do { loop_t ev; ptr_t<int> x ( 0UL );
                 ev.add([=](){ *x += 10; return -1; }); 
                 ev.next(); ev.next(); ev.next();
            if ( *x != 10 ){ break; }
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