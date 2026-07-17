#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/promise.h>

using namespace nodepp;
int x = 10;

promise_t<int,except_t> resolve(){
return promise_t<int,except_t>([=](
    res_t<int> res, rej_t<except_t> rej
){

    if( x-->0 ){ 

        timer::timeout([=](){
            res( resolve().await().value() );
        },1000);
        
    } else { res(100); }

}); }

void onMain(){ 

    resolve().then([=]( int res ){
        console::log( ">>", res );
    });

}