#define MAX_BATCH 10

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/atomic.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

atomic_t<int> worker_counter = 0;

void onWorker(){

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "hello world", worker_counter.add(1) );
        coDelay( rand() % 1000 ); process::clear(); }

    coFinish
    }));

}

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ 
        while( process::size() > MAX_BATCH ){ coDelay(100); }

            worker::add([=](){
                console::log( "--->", process::size() );
                onWorker ();
            process::wait(); return -1; });

        coNext; }

    coFinish
    }));

}

/*────────────────────────────────────────────────────────────────────────────*/
// BUGFIXED