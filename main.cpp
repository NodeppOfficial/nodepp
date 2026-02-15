#include <nodepp/nodepp.h>
#include <nodepp/cookie.h>

using namespace nodepp;

void onMain(){

    regex_t reg ( "\\d{3}-\\d{2}-\\d+" );

    console::log( reg.match( "123-12-123" ) );

}