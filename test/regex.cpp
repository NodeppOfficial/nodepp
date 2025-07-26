#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/regex.h>

using namespace nodepp;

namespace TEST { namespace REGEX {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | regex search", [](){
            try { auto out = regex::search_all( "hello 123 world!", "\\w+" );
             if ( out.size() != 3   ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | regex match", [](){
            try { auto out = regex::match_all( "hello 123 world!", "[^ ]+" );
             if ( out.size() != 3   ){ throw 0; }
             if ( out[0] != "hello" ){ throw 0; }
             if ( out[1] != "123"   ){ throw 0; }
             if ( out[2] != "world!"){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | regex memory", [](){
            try { auto out = regex::get_memory( "hello 123 world!", "([^ ]+)" );
             if ( out.size() != 3   ){ throw 0; }
             if ( out[0] != "hello" ){ throw 0; }
             if ( out[1] != "123"   ){ throw 0; }
             if ( out[2] != "world!"){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | regex format", [](){
            try { auto out = regex::format( "${0} ${1}", "hello", "world" );
             if ( out != "hello world" ){ throw 0; }
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