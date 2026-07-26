#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/map.h>

using namespace nodepp;

namespace TEST { namespace MAP {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | map initialization", [](){
            do {

                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
            if ( map.size() != 3 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | map searching", [](){
            do {

                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
            if ( !map.has("var1") ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | map indexation", [](){
            do {

                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
                map["var4"] = 40;
            if ( !map.has("var4") ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | map deindexation", [](){
            do {

                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
                 map.erase("var2");
            if ( map.has("var2") ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | map clearing", [](){
            do {

                map_t<string_t, int> map ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });

                  map.clear();
            if ( !map.empty() ){ break; }

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