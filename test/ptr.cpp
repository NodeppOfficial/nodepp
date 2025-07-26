#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace PTR {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | ptr initialization 1", [](){
            try {
                ptr_t<uint> arr ({ 10, 50, 90, 20 });
             if ( arr.size() != 4 ){ throw 0; }
             if ( arr[0]     !=10 ){ throw 0; }
             if ( arr[1]     !=50 ){ throw 0; }
             if ( arr[2]     !=90 ){ throw 0; }
             if ( arr[3]     !=20 ){ throw 0; } 
             if ( arr[4]     !=10 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 1 | ptr initialization 2", [](){
            try {
                ptr_t<uint> arr ( new uint(1000) );
             if ( *arr != 1000 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 12 | ptr clearing", [](){
            try {
                ptr_t<uint> arr ({ 10, 20, 30 }); 
                            arr = nullptr;
             if ( !arr.null() ){ throw 0; }
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