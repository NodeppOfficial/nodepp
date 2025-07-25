#include <nodepp/nodepp.h>
#include <nodepp/event.h>
#include <nodepp/timer.h>

using namespace nodepp;

void onMain(){

    event_t<> ev; // Event is task safety thanks to GC

    // create an event that can be emitted many times
    ev.on([](){ console::done(" World 1 "); });

    // create an event that can be emitted once
    ev.once([](){ console::done(" hello "); });

    // Emit Events
    ev.emit(); 
    
    // Clear Even Queue
    ev.clear();

    // turn of an specific event
    auto item = ev.on([](){  /* logic goes here */ });

    ev.off( item );


}