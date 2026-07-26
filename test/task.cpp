#include <nodepp/nodepp.h>
#include <nodepp/promise.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace TASK {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | async task testing", [](){
            do{ ptr_t<int> x ( 0UL ); 
                ptr_t<int> y = new int(3);

                process::add( coroutine::add( COROUTINE(){
                coBegin

                    while( *y>0 ){ *x += 10; *y-=1; coNext; }
                    
                coFinish
                }));

             while( *y!=0 ){ process::next(); }
                if( *x != 30 ){ TEST_FAIL (); }
                                TEST_DONE ();

            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | sync task testing", [](){
            do{ ptr_t<int> x ( 0UL ); 
                ptr_t<int> y = new int(3);

                process::await( coroutine::add( COROUTINE(){
                coBegin

                    while( *y>0 ){ *x += 10; *y-=1; coNext; }
                    
                coFinish
                }));

                if( *x != 30 ){ TEST_FAIL(); }
                                TEST_DONE();

            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | task resolver testing", [](){
            do{ ptr_t<int> x ( 0UL ); 
                ptr_t<int> y = new int(3);

                promise::resolve( coroutine::add( COROUTINE(){
                coBegin

                    while( *y>0 ){ *x += 10; *y-=1; coNext; }
                    
                coFinish })).emit();
                
                while( *y!=0 ){ process::next(); }
                if   ( *x != 30 ){ TEST_FAIL (); }
                                   TEST_DONE ();

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