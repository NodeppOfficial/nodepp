#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace STRING {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | string initialization", [](){
            do { string_t arr = "hello world!";
            if ( arr != "hello world!" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | string sorting", [](){
            do { string_t arr = "1092843756";
                 arr = arr.sort([=]( int a, int b ){ return a<=b; });
            if ( arr != "0123456789" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | string pop", [](){
            do { string_t arr = "0123"; arr.pop();
            if ( arr != "012" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | string shift", [](){
            do { string_t arr = "0123"; arr.shift();
            if ( arr != "123" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 5 | string unshift", [](){
            do { string_t arr = "0123"; 
                 arr.unshift( '9' );
            if ( arr != "90123" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 6 | string push", [](){
            do { string_t arr = "0123"; arr.push('9');
            if ( arr != "01239" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 7 | string concatenation", [](){
            do { string_t arr1 = "hello";
                 string_t arr2 = "world";
                 string_t arr3 = arr1 + arr2;
            if ( arr3 != "helloworld" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 8 | string slicing", [](){
            do { string_t arr1 = "hello world!";
                 string_t arr2 = arr1.slice( 0, 5 );
            if ( arr1 != "hello world!" ){ break; }
            if ( arr2 != "hello" )/*---*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 9 | string splicing 1", [](){
            do { string_t arr1 = "hello world!";
                 string_t arr2 = arr1.splice( 0, 5 );
            if ( arr1 !=" world!" ){ break; }
            if ( arr2 != "hello"  ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 10 | string splicing 2", [](){
            do { string_t arr1 = "hello world!";
                 arr1.splice( 5, 1, " aaa " );
            if ( arr1 != "hello aaa world!" ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 11 | string type::move", [](){
            do { string_t a = "aaa";
                 string_t b = type::move( a );
            if ( b.size() == 3 && a.empty() )
               { TEST_DONE(); } break;
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