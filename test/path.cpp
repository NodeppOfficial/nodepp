#include <nodepp/nodepp.h>
#include <nodepp/path.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace PATH {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | path mimetype", [](){
            try { string_t dir = path::join( os::cwd(), "www", "index.html" );
             if ( path::mimetype(dir) != "text/html" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | path basename", [](){
            try { string_t dir = path::join( os::cwd(), "www", "index.html" );
             if ( path::basename(dir,".html") != "index" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | path name", [](){
            try { string_t dir = path::join( os::cwd(), "www", "index.html" );
             if ( path::basename(dir) != "index.html" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | path extention", [](){
            try { string_t dir = path::join( os::cwd(), "www", "index.html" );
             if ( path::extname(dir) != "html" ){ throw 0; }
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