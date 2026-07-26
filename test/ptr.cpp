#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace PTR {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | ptr initialization 1", [](){
            do { ptr_t<uint> arr ({ 10, 50, 90, 20 });
            if ( arr.size() != 4 ){ break; }
            if ( arr[0]     !=10 ){ break; }
            if ( arr[1]     !=50 ){ break; }
            if ( arr[2]     !=90 ){ break; }
            if ( arr[3]     !=20 ){ break; } 
            if ( arr[4]     !=10 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 1 | ptr initialization 2", [](){
            do { ptr_t<uint> arr ( new uint(1000) );
            if ( *arr != 1000 ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 12 | ptr clearing", [](){
            do { ptr_t<uint> arr ({ 10, 20, 30 }); 
                             arr = nullptr;
            if ( !arr.null() ){ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 13 | ptr type::move", [](){
            do { ptr_t<uint> a (0UL); a[0] = 20;
                 ptr_t<uint> b = type::move( a );
            if ( !b.null() && a.null() )
               { TEST_DONE(); } break;
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 14 | ptr type::move", [](){
            do { ptr_t<uint> a ({ 10, 20, 30 });
                  ptr_t<uint> b = type::move( a );
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