#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/map.h>

using namespace nodepp;

namespace TEST { namespace MAP {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | map initialization", [](){
            try {
                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
             if ( map.size() != 3 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | map searching", [](){
            try {
                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
             if ( !map.has("var1") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | map indexation", [](){
            try {
                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
                map["var4"] = 40;
             if ( !map.has("var4") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | map deindexation", [](){
            try {
                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
                  map.erase("var2");
             if ( map.has("var2") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | map clearing", [](){
            try {
                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                }); map.clear();
             if ( !map.empty() ){ throw 0; }
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