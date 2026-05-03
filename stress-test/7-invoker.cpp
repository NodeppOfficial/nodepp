#include <nodepp/nodepp.h>

/*────────────────────────────────────────────────────────────────────────────*/

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    string_t x ( 32UL );
    string_t y = process::invoke([=]( any_t ){
        process::revoke( x ); // <- this must lead crash
        console::log( "hello world!" );
    return -1; }); // <- -1 important this force free to crash memory

    memcpy( x.get(), y.get(), y.size() );
    
    process::call( x, nullptr );
    process::call( x, nullptr );
    process::call( x, nullptr );

}

/*────────────────────────────────────────────────────────────────────────────*/