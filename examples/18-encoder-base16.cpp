#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void onMain() {

    string_t data = "hello world";
    string_t out  = encoder::buffer::atob( data );

    console::log( ">>", out );
    console::log( "<<", encoder::buffer::btoa( out ) );

}