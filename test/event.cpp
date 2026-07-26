#include <nodepp/nodepp.h>
#include <nodepp/event.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace EVENT {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | event initialization 1", [](){
            do { ptr_t<int> x ( 0UL );
                 event_t<> event; event.on([=](){ *x=1; });
                 event.emit();
            if ( event.empty() ){ break; }
            if ( *x==0 )/*----*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | event initialization 2", [](){
            do { ptr_t<int> x ( 0UL );
                 event_t<int> event; event.on([=]( int y ){ *x=y; });
                 event.emit(1);
            if ( event.empty() ){ break; }
            if ( *x==0 )/*----*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | once execution", [](){
            do { ptr_t<int> x ( 0UL );
                 event_t<> event; event.once([=](){ *x+=1; });
                 event.emit(); event.emit(); event.emit();
            if ( *x!=1 )/**/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | on execution", [](){
            do { ptr_t<int> x ( 0UL );
                 event_t<> event; event.on([=](){ *x+=1; });
                 event.emit(); event.emit(); event.emit();
            if ( event.empty() ){ break; }
            if ( *x!=3 )/*----*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 5 | skipping", [](){
            do { ptr_t<int> x ( 0UL );
                 event_t<> event; event.on([=](){ *x+=1; });
                 event.stop();   event.emit(); 
                 event.resume(); event.emit(); event.emit();
            if ( event.empty() ){ break; }
            if ( *x!=2 )/*----*/{ break; }
                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 6 | stopping", [](){
            do { ptr_t<int> x ( 0UL );
                 event_t<> event; event.on([=](){ *x+=1; });
                 event.stop(); event.emit(); 
                 event.emit(); event.emit();
            if ( event.empty() ){ break; }
            if ( *x!=0 )/*----*/{ break; }
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