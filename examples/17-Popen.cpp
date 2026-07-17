#include <nodepp/nodepp.h>
#include <nodepp/popen.h>

using namespace nodepp;

void onMain(){

    auto pid = popen::add( "curl https://www.google.com -Ls" );

    if( !pid.has_value() ){ 
        throw except_t( "something went wrong" );
    }

    pid.value().onData([]( string_t data ){ 
        console::log( data ); 
    });

    pid.value().onClose([=](){ 
        console::log("done"); 
    });

}