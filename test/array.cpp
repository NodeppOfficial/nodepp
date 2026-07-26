#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace ARRAY {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | array initialization", [](){
            do { array_t<uint> arr ({ 10, 50, 90, 20 });
            if ( arr.size() != 4 ){ break; }
            if ( arr[0]     !=10 ){ break; }
            if ( arr[1]     !=50 ){ break; }
            if ( arr[2]     !=90 ){ break; }
            if ( arr[3]     !=20 ){ break; } 
            if ( arr[4]     !=10 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | array sorting", [](){
            do { array_t<uint> arr ({ 10, 50, 90, 20 });
                 arr = arr.sort([=]( int a, int b ){ return a<=b; });
            if ( arr.size() != 4 ){ break; }
            if ( arr[0]     !=10 ){ break; }
            if ( arr[1]     !=20 ){ break; }
            if ( arr[2]     !=50 ){ break; }
            if ( arr[3]     !=90 ){ break; } 
            if ( arr[4]     !=10 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | array pop", [](){
            do { array_t<uint> arr ({ 10, 50, 90, 20 }); 
                 arr.pop();
            if ( arr.size() != 3 ){ break; }
            if ( arr[3]     !=10 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | array shift", [](){
            do { array_t<uint> arr ({ 10, 50, 90, 20 }); 
                 arr.shift();
            if ( arr.size() != 3 ){ break; }
            if ( arr[3]     !=50 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 5 | array unshift", [](){
            do { array_t<uint> arr ({ 10, 50, 90, 20 }); 
                 arr.unshift( 5 );
            if ( arr.size() != 5 ){ break; }
            if ( arr[0]     != 5 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 6 | array push", [](){
            do { array_t<uint> arr ({ 10, 50, 90, 20 }); 
                 arr.push( 5 );
            if ( arr.size() != 5 ){ break; }
            if ( arr[4]     != 5 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 7 | array concatenation 1", [](){
            do { array_t<uint> arr1 ({ 10, 20, 30 }); 
                 array_t<uint> arr2 ({ 40, 50, 60 }); 
                 arr1.insert( -1, arr2.size(), arr2.get() );
            if ( arr1.size()!= 6 ){ break; }
            if ( arr1[0]    !=10 ){ break; }
            if ( arr1[5]    !=60 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 8 | array concatenation 2", [](){
            do { array_t<uint> arr1 ({ 10, 20, 30 }); 
                 array_t<uint> arr2 ({ 40, 50, 60 }); 
                 arr1.insert( 0, arr2.size(), arr2.get() );
            if ( arr1.size()!= 6 ){ break; }
            if ( arr1[0]    !=40 ){ break; }
            if ( arr1[5]    !=30 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 9 | array slicing", [](){
            do { array_t<uint> arr ({ 10, 20, 30, 40, 50 });
                 auto tmp = arr.slice( 1, 3 );
            if ( tmp.empty() )/**/{ break; }
            if ( tmp.size() != 2 ){ break; }
            if ( arr.size() != 5 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 10 | array splicing 1", [](){
            do { array_t<uint> arr ({ 10, 20, 30, 40, 50 });
                 auto tmp = arr.splice( 1, 3 );
            if ( tmp.empty() )/**/{ break; }
            if ( tmp.size() != 3 ){ break; }
            if ( arr.size() == 5 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 11 | array splicing 2", [](){
            do { array_t<uint> arr ({ 10, 20, 30, 40, 50 });
                 arr.splice( 1, 3, { 2, 3, 4 } );
            if ( arr.empty() )/**/{ break; }
            if ( arr.size() != 5 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 12 | array clearing", [](){
            do { array_t<uint> arr ({ 10, 20, 30 }); arr.clear();
            if ( !arr.empty() ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 13 | array type::move", [](){
            do { array_t<uint> a ({ 10, 20, 30 });
                 array_t<uint> b = type::move( a );
            if ( b.size() == 3 && a.empty() )
               { TEST_DONE(); } TEST_FAIL();
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