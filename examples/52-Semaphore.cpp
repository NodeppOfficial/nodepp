#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/semaphore.h>

using namespace nodepp;

void onMain(){

    ptr_t<int> x = new int(100);
    semaphore_t sem;

    worker::add( coroutine::add( COROUTINE(){
        sem.wait( 0 );
    coBegin

        while( *x > 0 ){ *x-=1;
            console::info("Hello World 1:>",*x);
            coDelay( 100 ); sem.release(); 
        coNext; } sem.release();

    coFinish
    }));

    worker::add( coroutine::add( COROUTINE(){
        sem.wait( 1 );
    coBegin

        while( *x > 0 ){ *x-=1;
            console::done("Hello World 2:",*x);
            coDelay( 100 ); sem.release(); 
        coNext; } sem.release(); 

    coFinish
    }));

    worker::add( coroutine::add( COROUTINE(){
        sem.wait( 2 );
    coBegin

        while( *x > 0 ){ *x-=1;
            console::warning("Hello World 3:",*x);
            coDelay( 100 ); sem.release(); 
        coNext; } sem.release();

    coFinish
    }));


}