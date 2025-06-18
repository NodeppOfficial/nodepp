#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/semaphore.h>

using namespace nodepp;

void onMain(){

    ptr_t<int> x = new int(100);
    semaphore_t sem;

    worker::add([=](){ 
        sem.wait( 0 );
    coStart

        while( *x > 0 ){ *x-=1;
            console::info("Hello World 1:>",*x);
            worker::delay( 100 ); 
            sem.release(); coNext;
        }   sem.release(); 

    coStop
    });

    worker::add([=](){ 
        sem.wait( 1 );
    coStart

        while( *x > 0 ){ *x-=1;
            console::done("Hello World 2:",*x);
            worker::delay( 100 ); 
            sem.release(); coNext;
        }   sem.release(); 

    coStop
    });

    worker::add([=](){ 
        sem.wait( 2 );
    coStart

        while( *x > 0 ){ *x-=1;
            console::warning("Hello World 3:",*x);
            worker::delay( 100 ); 
            sem.release(); coNext;
        }   sem.release(); 

    coStop
    });

}