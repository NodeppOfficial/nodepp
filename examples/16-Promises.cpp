#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/promise.h>

using namespace nodepp;

void onMain(){

    promise_t<int,int>([=]( function_t<void,int> res, function_t<void,int> rej ){
        timer::timeout([=](){ res(10); },1000);
    })
    
    .then([=]( int res ){
        console::log("resolved:>",res);
    })
    
    .fail([=]( int rej ){
        console::log("rejected:>",rej);
    });

}