#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void onMain() {
    
    ulong data = 63499;
    auto  out  = encoder::hex::atob( data );

    console::log( ">>", out );
    console::log( "<<", encoder::hex::btoa<ulong>(out) );

}