#include <nodepp/nodepp.h>
#include <nodepp/wait.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace WAIT {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | wait initialization 1", [](){
            try { ptr_t<int> x = new int(0);
                  wait_t<string_t> wait; wait.on("test",[=](){ *x=1; });
                  wait.emit("test");
             if ( wait.empty() ){ throw 0; }
             if ( *x==0 )/*---*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | wait initialization 2", [](){
            try { ptr_t<int> x = new int(0);
                  wait_t<string_t,int> wait; wait.on("test",[=]( int y ){ *x=y; });
                  wait.emit("test",1);
             if ( wait.empty() ){ throw 0; }
             if ( *x==0 )/*---*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | once execution", [](){
            try { ptr_t<int> x = new int(0);
                  wait_t<string_t> wait; wait.once("test",[=](){ *x+=1; });
                  wait.emit("test"); wait.emit("test"); wait.emit("test");
             if ( wait.empty() ){ throw 0; }
             if ( *x!=1 )/*---*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | on execution", [](){
            try { ptr_t<int> x = new int(0);
                  wait_t<string_t> wait; wait.on("test",[=](){ *x+=1; });
                  wait.emit("test"); wait.emit("test"); wait.emit("test");
             if ( wait.empty() ){ throw 0; }
             if ( *x!=3 )/*---*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 5 | skipping", [](){
            try { ptr_t<int> x = new int(0);
                  wait_t<string_t> wait; wait.on("test",[=](){ *x+=1; });
                  wait.stop();   wait.emit("test"); 
                  wait.resume(); wait.emit("test"); wait.emit("test");
             if ( wait.empty() ){ throw 0; }
             if ( *x!=2 )/*---*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 6 | stopping", [](){
            try { ptr_t<int> x = new int(0);
                  wait_t<string_t> wait; wait.on("test",[=](){ *x+=1; });
                  wait.stop();       wait.emit("test"); 
                  wait.emit("test"); wait.emit("test");
             if ( wait.empty() ){ throw 0; }
             if ( *x!=0 )/*---*/{ throw 0; }
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