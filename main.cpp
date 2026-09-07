#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void onMain(){

    auto x = 'p';
    auto a = encoder::hex::atob      ( x );
    auto b = encoder::hex::btoa<char>( a );

    console::log( a, x );

}