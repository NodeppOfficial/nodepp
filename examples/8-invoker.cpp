#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain() {

    invoke_t<int> ivk;

    uchar_64 addr = ivk.add([=]( int value ){
        console::log( ">>", value );
    return 1; });

    ivk.emit( addr, 10 );
    ivk.emit( addr, 20 );
    ivk.emit( addr, 30 );

}