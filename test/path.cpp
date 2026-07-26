#include <nodepp/nodepp.h>
#include <nodepp/path.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace PATH {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | path mimetype", [](){
            do { string_t dir = path::join( os::cwd(), "www", "index.html" );
            if ( path::mimetype(dir) != "text/html" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | path basename", [](){
            do { string_t dir = path::join( os::cwd(), "www", "index.html" );
            if ( path::basename(dir,".html") != "index" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | path name", [](){
            do { string_t dir = path::join( os::cwd(), "www", "index.html" );
            if ( path::basename(dir) != "index.html" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | path extention", [](){
            do { string_t dir = path::join( os::cwd(), "www", "index.html" );
            if ( path::extname(dir) != "html" ){ break; }
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