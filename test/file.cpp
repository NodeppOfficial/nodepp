#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/file.h>
#include <nodepp/fs.h>

using namespace nodepp;

namespace TEST { namespace FILE {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | file write", [](){
            do { file_t file( "test_file", "w" );
                 file.write ( "hello world!" );
            if ( !fs::exists_file( "test_file" ) )
               { break; }   TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | file read", [](){
            do { file_t file( "test_file", "r" );
                 auto data = file.read();
            if ( data != "hello world!" ) 
               { break; }   TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | file delete", [](){
            do { fs::remove_file( "test_file" );
            if ( fs::exists_file( "test_file" ) ){ break; }  
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