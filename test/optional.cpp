#include <nodepp/nodepp.h>
#include <nodepp/optional.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace OPTION {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | optional -> done", [](){
            do {

                optional_t<string_t> x;
                if( x.has_value() ){ TEST_FAIL(); }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | optional -> error", [](){
            do {

                optional_t<string_t> x ( "hello world!" );
                if( !x.has_value() ){ TEST_FAIL(); }

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