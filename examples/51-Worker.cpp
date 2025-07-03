#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    ptr_t<int> x = new int(100);
    mutex_t mut;

    worker::add( coroutine::add( COROUTINE(){
        mut.lock();
    coBegin

        while( *x > 0 ){ *x-=1;
            console::info("Hello World",*x);
            coDelay( 100 ); mut.unlock(); 
            coNext;
        }

    coFinish
    }));

    auto wrk = worker::add( coroutine::add( COROUTINE(){
        mut.lock();
    coBegin

        while( *x > 0 ){ *x-=1;
            console::done("Hello World",*x);
            coDelay( 100 ); mut.unlock(); 
            coNext;
        }

    coFinish
    }));

    timer::timeout([=](){ wrk.off(); }, 1000 );

}