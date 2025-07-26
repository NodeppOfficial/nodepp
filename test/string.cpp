#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace STRING {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | string initialization", [](){
            try { string_t arr = "hello world!";
             if ( arr != "hello world!" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | string sorting", [](){
            try { string_t arr = "1092843756";
                  arr = arr.sort([=]( int a, int b ){ return a<=b; });
             if ( arr != "0123456789" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | string pop", [](){
            try { string_t arr = "0123"; arr.pop();
             if ( arr != "012" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | string shift", [](){
            try { string_t arr = "0123"; arr.shift();
             if ( arr != "123" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 5 | string unshift", [](){
            try { string_t arr = "0123"; 
                  arr.unshift( '9' );
             if ( arr != "90123" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 6 | string push", [](){
            try { string_t arr = "0123"; arr.push('9');
             if ( arr != "01239" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 7 | string concatenation", [](){
            try { string_t arr1 = "hello";
                  string_t arr2 = "world";
                  string_t arr3 = arr1 + arr2;
             if ( arr3 != "helloworld" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 8 | string slicing", [](){
            try { string_t arr1 = "hello world!";
                  string_t arr2 = arr1.slice( 0, 5 );
             if ( arr1 != "hello world!" ){ throw 0; }
             if ( arr2 != "hello" )/*---*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 9 | string splicing 1", [](){
            try { string_t arr1 = "hello world!";
                  string_t arr2 = arr1.splice( 0, 5 );
             if ( arr1 !=" world!" ){ throw 0; }
             if ( arr2 != "hello"  ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 10 | string splicing 2", [](){
            try { string_t arr1 = "hello world!";
                  arr1.splice( 5, 1, " aaa " );
             if ( arr1 != "hello aaa world!" ){ throw 0; }
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