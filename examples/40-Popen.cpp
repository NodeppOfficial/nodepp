#include <nodepp/nodepp.h>
#include <nodepp/popen.h>

using namespace nodepp;

void onMain(){

    auto pid = popen::async( "curl https://www.google.com -Ls" );

    pid.onData([]( string_t data ){ console::log( data ); });
    pid.onClose([=](){ console::log("done"); });

}