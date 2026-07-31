#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace CONSOLE {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | console log ", [](){ do {
            conio::log("-> "); console::log("Hello World!");
                    TEST_DONE();
        } while(0); TEST_FAIL(); });

        TEST_ADD( test, "TEST 2 | console info ", [](){ do {
            conio::log("-> "); console::info("Hello World!");
                    TEST_DONE();
        } while(0); TEST_FAIL(); });

        TEST_ADD( test, "TEST 3 | console done ", [](){ do {
            conio::log("-> "); console::done("Hello World!");
                    TEST_DONE();
        } while(0); TEST_FAIL(); });

        TEST_ADD( test, "TEST 4 | console error ", [](){ do {
            conio::log("-> "); console::error("Hello World!");
                    TEST_DONE();
        } while(0); TEST_FAIL(); });

        TEST_ADD( test, "TEST 5 | console success ", [](){ do {
            conio::log("-> "); console::success("Hello World!");
                    TEST_DONE();
        } while(0); TEST_FAIL(); });

        TEST_ADD( test, "TEST 6 | console warning ", [](){ do {
            conio::log("-> "); console::warning("Hello World!");
                    TEST_DONE();
        } while(0); TEST_FAIL(); });

        test.onClose.once([=](){
            console::log(":> RESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err)++;  (*totl)++; });
        test.onSkip([=](){ (*skp)++;  (*totl)++; });

        TEST_AWAIT( test );

    }

}}

void onMain(){ TEST::CONSOLE::TEST_RUNNER(); }