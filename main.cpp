#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

void onMain(){

    event_t<> ev;

    ev.once([=](){ console::log( "hello once" ); });
    ev.on  ([=](){
        console::log( "hello world" );
    ev.emit(); });

    ev.emit();
    ev.emit();
    ev.emit();

}