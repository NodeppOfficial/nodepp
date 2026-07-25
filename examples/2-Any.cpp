#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    any_t value = ptr_t<uchar_64>( 0UL );

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            console::log( ">>", *value.as<ptr_t<uchar_64>>() );
        coDelay(1000); }

    coFinish
    }));

    process::add( coroutine::add( COROUTINE(){
    coBegin

        while( true ){
            value.as<ptr_t<uchar_64>>()[0] = process::now();
        coDelay(1000); }

    coFinish
    }));

}