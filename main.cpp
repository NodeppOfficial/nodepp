#include <nodepp/nodepp.h>
#include <nodepp/listener.h>

using namespace nodepp;

void onMain(){

    listener_t< string_t, any_t > ev;

    auto addr = process::invoke([=]( any_t val ){
         console::log( ">>", val.has_value() );
         ev.emit( "mojon", nullptr );
    return 1; });

    ev.on( "mojon", [=]( any_t raw ){ 
        process::call( addr, raw );
    });

    ev.emit( "mojon", nullptr );
    ev.emit( "mojon", nullptr );
    ev.emit( "mojon", nullptr );

}