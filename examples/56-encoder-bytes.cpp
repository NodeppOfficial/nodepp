#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void onMain() {
    
    ulong data = 63499;
    auto  out  = encoder::bytes::atob( data );

    console::log( ">>", array_t<uchar>(out).join() );
    console::log( "<<", encoder::bytes::btoa<ulong>(out) );

}