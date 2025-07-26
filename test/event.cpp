#include <nodepp/nodepp.h>
#include <nodepp/event.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace EVENT {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | event initialization 1", [](){
            try { ptr_t<int> x = new int(0);
                  event_t<> event; event.on([=](){ *x=1; });
                  event.emit();
             if ( event.empty() ){ throw 0; }
             if ( *x==0 )/*----*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | event initialization 2", [](){
            try { ptr_t<int> x = new int(0);
                  event_t<int> event; event.on([=]( int y ){ *x=y; });
                  event.emit(1);
             if ( event.empty() ){ throw 0; }
             if ( *x==0 )/*----*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | once execution", [](){
            try { ptr_t<int> x = new int(0);
                  event_t<> event; event.once([=](){ *x+=1; });
                  event.emit(); event.emit(); event.emit();
             if ( event.empty() ){ throw 0; }
             if ( *x!=1 )/*----*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | on execution", [](){
            try { ptr_t<int> x = new int(0);
                  event_t<> event; event.on([=](){ *x+=1; });
                  event.emit(); event.emit(); event.emit();
             if ( event.empty() ){ throw 0; }
             if ( *x!=3 )/*----*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 5 | skipping", [](){
            try { ptr_t<int> x = new int(0);
                  event_t<> event; event.on([=](){ *x+=1; });
                  event.stop();   event.emit(); 
                  event.resume(); event.emit(); event.emit();
             if ( event.empty() ){ throw 0; }
             if ( *x!=2 )/*----*/{ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 6 | stopping", [](){
            try { ptr_t<int> x = new int(0);
                  event_t<> event; event.on([=](){ *x+=1; });
                  event.stop(); event.emit(); 
                  event.emit(); event.emit();
             if ( event.empty() ){ throw 0; }
             if ( *x!=0 )/*----*/{ throw 0; }
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