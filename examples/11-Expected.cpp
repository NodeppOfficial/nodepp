#include <nodepp/nodepp.h>
#include <nodepp/expected.h>

using namespace nodepp;

void onMain(){

    expected_t<int,string_t> x ( 10 );
    if( x.has_value() ) console::done( x.value() );
    else                console::error( x.error() );

    expected_t<int,string_t> y ( "hello" );
    if( y.has_value() ) console::done( y.value() );
    else                console::error( y.error() );

}