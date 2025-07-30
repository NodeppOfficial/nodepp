#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/url.h>

using namespace nodepp;

namespace TEST { namespace URL {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | url hostname", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::hostname(uri) != "www.google.com" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | url port", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::port(uri) != 80 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | url protocol", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::protocol(uri) != "http" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | url pathname", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::path(uri) != "/path/to/file" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 5 | url search", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::search(uri) != "?var1=10&var2=50&var3=100&var4=hello_world!" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 6 | url hash", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::hash(uri) != "#done" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 7 | url origin", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::origin(uri) != "http://becerra:enmanuel@www.google.com" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 8 | url auth", [](){
            try { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
             if ( url::auth(uri) != "becerra:enmanuel" ){ throw 0; }
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