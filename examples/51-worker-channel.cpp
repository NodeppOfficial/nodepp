#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/channel.h>

using namespace nodepp;

void onMain(){

    channel_t<string_t> ch1; 

    worker::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            coWait( ch1._write( "hello world" )==-2 );
        coDelay(1000); }

    coFinish
    }) );

    worker::add( coroutine::add( COROUTINE(){
        ptr_t<string_t> tmp;
    coBegin

        while( true ){
            coWait( ch1._read( tmp )==-2 );
            console::log( *tmp );
        coDelay(1000); }

    coFinish
    }) );

}