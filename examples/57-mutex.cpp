#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    mutex_t mut;

    worker::add( coroutine::add([=](){
    coBegin

        while( true ){ mut.lock([=](){
            // executed in critical section method A
        }); coNext; }

    coFinish
    }));

    worker::add( coroutine::add([=](){
    coBegin

        while( true ){ coWait( mut._lock([=](){
            // executed in critical section method B
        })==-2 );
            // executed out critical section 
        coNext; }

    coFinish
    }));

    worker::add( coroutine::add([=](){
    coBegin

        while( true ){ mut.lock();
            // executed in critical section method C
        mut.unlock(); coNext; }

    coFinish
    }));

    worker::add( mutex::add( mut, coroutine::add([=](){
    coBegin

        while( true ){
            // executed in critical section method D
        coNext; }

    coFinish
    }) ));

}