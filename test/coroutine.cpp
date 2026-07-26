#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace COROUTINE {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | coroutine (simple)", [](){
            do { ptr_t<int> x ( 0UL );
                
                auto co = coroutine::add( COROUTINE(){ 
                     *x = 100;
                return -1; }); co();

            if ( *x != 100 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | coroutine (Argument)", [](){
            do { ptr_t<int> x ( 0UL );
                
                auto co = coroutine::add<int>( COROUTINE_ARG( int arg ){ 
                     *x = arg;
                return -1; }); co( 10 );

            if ( *x != 10 ){ break; }
            
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