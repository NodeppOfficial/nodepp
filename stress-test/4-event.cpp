#include <nodepp/nodepp.h>
#include <nodepp/event.h>

using namespace nodepp;

void onMain(){

    event_t<> ev;
    event_t<> ov;

    auto addr = ev.on([=](){ console::log( "-A-" ); });
    /*-------*/ ev.on([=](){ console::log( "-B-" ); });

    /* // CASE 1 
    ev.on([=](){ 
        ev.clear(); // <- this must crash the programs | bug fixed :)
        ev.on([=](){ console::log( "hello world" ); });
        console::log( "<---->", ev.size() );
    });
    */

    /* // CASE 2
    ev.on([=](){ ov.once([=](){
        ev.clear(); // <- this must crash the programs | bug fixed :)
        ev.on([=](){ console::log( "hello world" ); });
        console::log( "<---->", ev.size() );
    }); ov.emit(); });
    */

    /**/ // CASE 3
    ev.once([=](){ ov.emit(); }); // <- this must brake the event queue
    ov.on  ([=](){ ev.off( addr ); });
    /**/

    ev.on([=](){ console::log( "-C-" ); });
    ev.on([=](){ console::log( "-D-" ); });

    ev.emit();
    ev.emit();
    ev.emit();
    ev.emit();
    
    console::log( "EEE" );

}