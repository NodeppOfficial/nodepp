#include <nodepp/nodepp.h>
#include <nodepp/popen.h>

using namespace nodepp;

void onMain(){

    array_t<string_t> cmd ({ "curl", "http://www.google.com/", "-Ls" });
    auto pid = popen::async( cmd[0], cmd );

    pid.onData([]( string_t data ){ console::log( data ); });
    pid.onClose([=](){ console::log("done"); });

}