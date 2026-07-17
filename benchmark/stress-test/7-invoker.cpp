#define MAX_BATCH 10

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain(){

    ptr_t<uchar_64> x ( 0UL );

    uchar_64 y = process::invoke([=]( any_t ){
        process::revoke( *x ); // <- this must lead crash
        console::log( "hello world!" );
    return -1; }); // <- -1 important this force free to crash memory

    *x = y;

    process::call( y, nullptr );
    process::call( y, nullptr );
    process::call( y, nullptr );

}

/*────────────────────────────────────────────────────────────────────────────*/
// BUGFIXED