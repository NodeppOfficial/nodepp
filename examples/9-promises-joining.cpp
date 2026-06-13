#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/promise.h>

using namespace nodepp;

promise_t<int,except_t> resolve(){
return promise_t<int,except_t> ([=](
       res_t<int> res, rej_t<except_t> rej
){

    timer::timeout([=](){ res(10); },1000);

}); }

void onMain(){

    promise_t<int,except_t>([=]( res_t<int> res, rej_t<except_t> rej ){
    resolve().join( res, rej ); })

    .then([=]( int res ){
        console::log("resolved:>",res);
    })

    .fail([=]( except_t rej ){
        console::log("rejected:>",rej);
    })

    .finally([=](){
        console::log("--finally--");
    });

}
