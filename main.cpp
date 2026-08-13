#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){ 

    auto tmp  = ptr_t<uchar_64>( 0UL );
    auto addr = process::invoke([=]( any_t ){

        process::delay(1000);
        console::log( "BBBB", *tmp );
        process::call( *tmp, nullptr );

    return -1; }); *tmp = addr;

    console::log( "AAAA", *tmp );
    process::call( *tmp, nullptr );

}