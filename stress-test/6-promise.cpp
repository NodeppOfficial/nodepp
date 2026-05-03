#include <nodepp/nodepp.h>
#include <nodepp/promise.h>

using namespace nodepp;

void resolve(){

    promise_t<null_t,except_t> ([=](
        res_t<null_t> res, rej_t<except_t> rej
    ){

        process::add( coroutine::add( COROUTINE(){
        coBegin

            coDelay( 10 ); res( nullptr );
            resolve(); // <- this must create a memory leak | bug fixed :)

        coFinish
        }));

    })

    .then([=]( null_t ){
        console::log( "done" );
    })

    .fail([=]( except_t ){
        console::log( "error" );
    });
    
}

void onMain(){ resolve(); }