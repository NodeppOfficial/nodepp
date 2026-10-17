#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/channel.h>

using namespace nodepp;

channel_t<coroutine_t> ch1, ch2;

int worker_1(){
coStart

    while( true ){ coDelay(1000); 
        
        ch2.write( coroutine_t( COROUTINE(){
            console::log( "worker1 -> worker2", process::now() );
        return -1; }) );

    for( auto x: ch1.read() ){ process::add( x ); }}

coStop
}

int worker_2(){
coStart

    while( true ){ coDelay(1000); 
        
        ch1.write( coroutine_t( COROUTINE(){
            console::log( "worker2 -> worker1", process::now() );
        return -1; }) );

    for( auto x: ch2.read() ){ process::add( x ); }}

coStop
}

void onMain(){

    worker ::add([=](){
    process::add([=](){ return worker_1(); });
    process::wait(); return -1; });

    worker ::add([=](){
    process::add([=](){ return worker_2(); });
    process::wait(); return -1; });

}