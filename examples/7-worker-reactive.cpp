#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/promise.h>

using namespace nodepp;

int task( int* value ) {
coStart

    while( *value>0 ){
        console::log( "waiting", *value );
    *value -= 1; coDelay(1000); }

coStop
}

void onMain(){

    promise_t<string_t,except_t> ([=](
        res_t<string_t> res, rej_t<except_t> rej
    ){

        ptr_t<int> value ( 0UL, 10 );

        worker::add([=](){
            while( task( &value )==1 ){ return 1; }
            res( "hello world!" );
        return -1; });

    })

    .then([=]( string_t data ){
        console::done( data );
    })

    .fail([=]( except_t err ){
        console::error( err.what() );
    });

}