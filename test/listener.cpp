#include <nodepp/nodepp.h>
#include <nodepp/listener.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace LISTENER {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | wait initialization 1", [](){
            do { ptr_t<int> x ( 0UL );
                 listener_t<string_t> wait; wait.on("test",[=](){ *x=1; });
                 wait.emit("test");
            if ( wait.empty() ){ break; }
            if ( *x==0 )/*---*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | wait initialization 2", [](){
            do { ptr_t<int> x ( 0UL );
                 listener_t<string_t,int> wait; wait.on("test",[=]( int y ){ *x=y; });
                 wait.emit("test",1);
            if ( wait.empty() ){ break; }
            if ( *x==0 )/*---*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | once execution", [](){
            do { ptr_t<int> x ( 0UL );
                 listener_t<string_t> wait; wait.once("test",[=](){ *x+=1; });
                 wait.emit("test"); wait.emit("test"); wait.emit("test");
            if ( *x!=1 )/**/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | on execution", [](){
            do { ptr_t<int> x ( 0UL );
                 listener_t<string_t> wait; wait.on("test",[=](){ *x+=1; });
                 wait.emit("test"); wait.emit("test"); wait.emit("test");
            if ( wait.empty() ){ break; }
            if ( *x!=3 )/*---*/{ break; }
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