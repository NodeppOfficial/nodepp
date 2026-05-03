#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/channel.h>

using namespace nodepp;

channel_t<coroutine_t> ch1, ch2;

int worker_1(){
coStart

    while( true ){
        
        ch2.write( coroutine_t( COROUTINE(){
            console::log( "worker1 -> worker2", process::now() );
        return -1; }) );

    if ( !ch1.empty() ){ 
    for( auto x: ch1.read() ){
         process::add( x );
    }}

    coDelay(1000); }

coStop
}

int worker_2(){
coStart

    while( true ){
        
        ch1.write( coroutine_t( COROUTINE(){
            console::log( "worker2 -> worker1", process::now() );
        return -1; }) );

    if ( !ch2.empty() ){ 
    for( auto x: ch2.read() ){
         process::add( x );
    }}

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