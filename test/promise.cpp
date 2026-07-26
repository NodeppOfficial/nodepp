#include <nodepp/nodepp.h>
#include <nodepp/promise.h>
#include <nodepp/timer.h>

using namespace nodepp;

namespace TEST { namespace PROMISE {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | promise (then) async", [](){
            do{ ptr_t<int> x ( 0UL ); ptr_t<bool> y ( 0UL );

                promise_t<int,except_t> ([=]( res_t<int> res, rej_t<except_t> rej ){
                  timer::timeout([=](){ res(100); *y=1; },1000);
                })

                .then([=]( int /**/ res ){ *x = res; });

                while( *y==0 ){ process::next(); }
                if( *x !=100 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | promise (fail) async", [](){
            do{ ptr_t<int> x ( 0UL ); ptr_t<bool> y ( 0UL );

                promise_t<int,except_t> ([=]( res_t<int> res, rej_t<except_t> rej ){
                  timer::timeout([=](){ rej( "error" ); *y=1; },1000);
                })

                .then([=]( int  res ){ *x = res; })
                .fail([=]( except_t ){ *x = 100; });

                while( *y==0 ){ process::next(); }
                if( *x !=100 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | promise chaining async", [](){
            do{ ptr_t<int> x ( 0UL ); ptr_t<bool> y ( 0UL );

                promise_t<int,except_t> ([=]( res_t<int> res, rej_t<except_t> rej ){
                  timer::timeout([=](){ res( 100 ); *y=1; },1000);
                })

                .then([=]( int res ){ *x += res; })
                .then([=]( int res ){ *x += res; })
                .then([=]( int res ){ *x += res; })
                .then([=]( int res ){ *x += res; });

                while( *y==0 ){ process::next(); }
                if ( *x!=400 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | promise (finally) async", [](){
            do{ ptr_t<int> x ( 0UL ); ptr_t<bool> y ( 0UL );

                promise_t<int,except_t> ([=]( res_t<int> res, rej_t<except_t> rej ){
                timer::timeout([=](){ rej( "error" ); *y=1; },1000); })

                .then   ([=]( int res ){ *x = res; })
                .finally([=]( /*---*/ ){ *x = 100; });

                while( *y==0 ){ process::next(); }
                if( *x !=100 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 5 | promise (then) sync", [](){
            do {

                auto data = promise_t<int,except_t>([=]( res_t<int> res, rej_t<except_t> rej ){
                  timer::timeout([=](){ res(100); },1000);
                }).await();

                if( !data.has_value() ){ break; }
                if( data.value()!=100 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 6 | promise (fail) sync", [](){
            do {

                auto data = promise_t<int,except_t>([=]( res_t<int> res, rej_t<except_t> rej ){
                  timer::timeout([=](){ rej( "error" ); },1000);
                }).await();

                if( data.has_value() ){ break; }

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