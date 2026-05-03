#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    mutex_t mut;

    worker::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ 
        
        mut.lock([=](){
            // executed in critical section method A1
            // lock will block this thread until get in critical section 
            console::log( "worker running critical section method A1" );
        });
            
        coWait( mut._lock([=](){
            // executed in critical section method A2
            // _lock uses try_lock() under the hood, so this will NOT block
            // instead, it will re-schedule this task untill can get in 
            // critical section.
            console::log( "worker running critical section method A2" );
        })==-2 );

        coNext; }

    coFinish
    }));

    worker::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ mut.lock();
            // executed in critical section method B
            console::log( "worker running critical section method B" );
        mut.unlock(); coNext; }

    coFinish
    }));

    worker::add( mutex::add( mut, coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            // executed in critical section method C
            // the whole coroutine is executed in critical section
            console::log( "worker running critical section method C" );
        coNext; }

    coFinish
    }) ));

}