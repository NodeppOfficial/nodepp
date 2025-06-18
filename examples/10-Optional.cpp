#include <nodepp/nodepp.h>
#include <nodepp/optional.h>

using namespace nodepp;

void onMain(){

    optional_t<string_t> x;
    if( x.has_value() ) console::done( x.value() );
    else                console::error( "x is empty" );

    optional_t<string_t> y ("y has value");
    if( y.has_value() ) console::done( y.value() );
    else                console::error( "y is empty" );

}