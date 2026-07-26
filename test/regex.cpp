#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/regex.h>

using namespace nodepp;

namespace TEST { namespace REGEX {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | regex search", [](){
            do { auto out = regex::search_all( "hello 123 world!", "\\w+" );
            if ( out.size() != 3 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | regex match", [](){
            do { auto out = regex::match_all( "hello 123 world!", "[^ ]+" );
            if ( out.size() != 3   ){ break; }
            if ( out[0] != "hello" ){ break; }
            if ( out[1] != "123"   ){ break; }
            if ( out[2] != "world!"){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | regex memory", [](){
            do { auto out = regex::get_memory( "hello 123 world!", "([^ ]+)" );
            if ( out.size() != 3   ){ break; }
            if ( out[0] != "hello" ){ break; }
            if ( out[1] != "123"   ){ break; }
            if ( out[2] != "world!"){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | regex format", [](){
            do { auto out = regex::format( "${0} ${1}", "hello", "world" );
            if ( out != "hello world" ){ break; }
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