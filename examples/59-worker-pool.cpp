#include <nodepp/nodepp.h>
#include <nodepp/wpool.h>

using namespace nodepp;

void onMain(){

    wpool_t wpool; // or wpool_t wpool( pool_size ) <- default MAX_POOL_SIZE

    wpool.add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "-> task 1" );
        coDelay(10); }

    coFinish
    }));

    wpool.add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( "-> task 2" );
        coDelay(1000); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while  ( !wpool.empty()    ){           // block the coroutine
        while  ( wpool.next()>=0   ){ coNext; } // execute all task asynchronously
        coDelay( wpool.get_delay() ); }         // delay the task using the lowest timer detected by wpool

    coFinish
    }));

}