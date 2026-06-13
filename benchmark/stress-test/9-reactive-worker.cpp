#define MAX_BATCH 10

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/atomic.h>
#include <nodepp/promise.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    atomic_ptr_t<ulong> ptr ( new ulong(0UL) );

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){ 
        while( process::size() > MAX_BATCH ){ coDelay(1000); }
        do {
        
            promise_t<int,except_t> prs ([=](
                res_t<int> res, rej_t<except_t> rej
            ){

                worker::add( coroutine::add( COROUTINE(){
                coBegin
                
                    coDelay( (rand()%1000) + 1 );
                    res( (*ptr)++ ); // <- this must cause race-condition

                coFinish
                }));
            //  ^ and the worker itself  

            });

            prs.then([=]( int val ){
                console::log( "hello world", val );
            });

        } while(0); coNext; }

    coFinish
    }));

}

/*────────────────────────────────────────────────────────────────────────────*/
// BUGFIXED