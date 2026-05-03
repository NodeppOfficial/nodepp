#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/event.h>

using namespace nodepp;

event_t<coroutine_t> ev1, ev2;

int worker_1(){
    static queue_t<coroutine_t> que;
    static mutex_t /*--------*/ mut;
coStart

    ev1.on([=]( coroutine_t clb ){ mut.lock([&](){
        que.push( clb );
    }); });

    while( true ){
        
        ev2.emit( coroutine_t( COROUTINE(){
            console::log( "worker1 -> worker2", process::now() );
        return -1; }) );

    if(!que.empty() ){ mut.lock([&](){
        que.map([=]( coroutine_t clb ){
            process::add( clb );
        }); que.clear();
    }); }

    coDelay(1000); }

coStop
}

int worker_2(){
    static queue_t<coroutine_t> que;
    static mutex_t /*--------*/ mut;
coStart

    ev2.on([=]( coroutine_t clb ){ mut.lock([&](){
        que.push( clb );
    }); });

    while( true ){
        
        ev1.emit( coroutine_t( COROUTINE(){
            console::log( "worker2 -> worker1", process::now() );
        return -1; }) );

    if(!que.empty() ){ mut.lock([&](){
        que.map([=]( coroutine_t clb ){
            process::add( clb );
        }); que.clear();
    }); }

    coDelay(1000); }

coStop
}

void onMain(){

    worker::add([=](){
        process::add([=](){ return worker_1(); });
    process::wait();
    return -1; });

    worker::add([=](){
        process::add([=](){ return worker_2(); });
    process::wait();
    return -1; });

}