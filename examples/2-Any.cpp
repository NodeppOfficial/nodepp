#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    any_t value = 10UL;

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( ">>", value.as<ulong>() );
        coDelay(1000); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            value.as<ulong>() = process::now();
        coDelay(1000); }

    coFinish
    }));

}