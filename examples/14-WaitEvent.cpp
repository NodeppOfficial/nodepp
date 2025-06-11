#include <nodepp/nodepp.h>
#include <nodepp/timer.h>
#include <nodepp/wait.h>

using namespace nodepp;

void onMain(){

    wait_t<string_t> ev;

    ev.on("1",[](){ console::log(" World 1 "); });
    ev.on("2",[](){ console::log(" World 2 "); });
    ev.on("3",[](){ console::log(" World 3 "); });

    ev.emit( "3" );
    ev.emit( "2" );
    ev.emit( "1" );

}