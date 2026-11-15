#define MAX_BATCH 10

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/promise.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void resolve(){

    promise_t<any_t,except_t> ([=](
        res_t<any_t> res, rej_t<except_t> rej
    ){

        process::add( coroutine::add( COROUTINE(){
        coBegin

            coDelay( 10 ); res( 0x00 );
            resolve(); // <- this must create a memory leak | bug fixed :)

        coFinish
        }));

    })

    .then([=]( any_t    ){ console::log( "done"  ); })
    .fail([=]( except_t ){ console::log( "error" ); });
    
}

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){ resolve(); }

/*────────────────────────────────────────────────────────────────────────────*/
// BUGFIXED