#include <nodepp/nodepp.h>
#include <nodepp/loop.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace LOOP {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | EVloop initialization", [](){
            try { loop_t ev; ptr_t<int> x = new int(0);
                  ev.add([=](){ *x = 10; return -1; }); 
                  ev.next();
             if ( *x != 10   ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 1 | EVloop iteration 1", [](){
            try { loop_t ev; ptr_t<int> x = new int(0);
                  ev.add([=](){ *x += 10; return 1; }); 
                  ev.next(); ev.next(); ev.next();
             if ( *x != 30   ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 1 | EVloop iteration 2", [](){
            try { loop_t ev; ptr_t<int> x = new int(0);
                  ev.add([=](){ *x += 10; return -1; }); 
                  ev.next(); ev.next(); ev.next();
             if ( *x != 10   ){ throw 0; }
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