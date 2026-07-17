#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void onMain() {
    
    ulong data = 63499;
    auto  out  = encoder::bin::atob( data );

    console::log( ">>", array_t<bool>(out).join("") );
    console::log( "<<", encoder::bin::btoa<ulong>(out) );

}