#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/url.h>

using namespace nodepp;

namespace TEST { namespace URL {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | url hostname", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::hostname(uri) != "www.google.com" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | url port", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::port(uri) != 80 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | url protocol", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::protocol(uri) != "http" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | url pathname", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::path(uri) != "/path/to/file" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 5 | url search", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::search(uri) != "?var1=10&var2=50&var3=100&var4=hello_world!" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 6 | url hash", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::hash(uri) != "#done" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 7 | url origin", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::origin(uri) != "http://becerra:enmanuel@www.google.com" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 8 | url auth", [](){
            do { string_t uri = "http://becerra:enmanuel@www.google.com/path/to/file#done?var1=10&var2=50&var3=100&var4=hello_world!";
            if ( url::auth(uri) != "becerra:enmanuel" ){ break; }
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