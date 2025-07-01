#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void onMain() {

    string_t data = "hello world";
    string_t out  = encoder::base64::atob( data );

    console::log( ">>", out );
    console::log( "<<", encoder::base64::btoa( out ) );

}