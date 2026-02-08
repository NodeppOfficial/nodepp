#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/promise.h>

using namespace nodepp;

void onMain(){

    promise_t<int,except_t> prom ([=]( res_t<int> res, rej_t<except_t> rej ){
        timer::timeout([=](){ res(10); },1000);
    }); prom.emit();

    process::add( coroutine::add( COROUTINE(){
    coBegin

        coWait( prom.is_pending() );

        if( !prom.has_value()  ){ coEnd; }
        
        if( prom.is_resolved() ){ console::done ( prom.get_value().value() ); }
        else /*--------------*/ { console::error( prom.get_value().error() ); }
        
    coFinish
    }));

}
