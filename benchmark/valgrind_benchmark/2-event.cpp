#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain() {

    console::log("Starting Lifecycle Stress Test...");

    for( int i=0; i<1000000; i++ ){

        // Create a smart pointer and an event
        ptr_t  <int> data ( 0UL, i );
        event_t<int> ev;

        // Add a listener (this tests the internal queue/lambda storage)
        ev.on([=]( int val ){ int trash = *data + val; });

        if( i % 250000 == 0 ){ console::log("Processed:", i); }
        // data and ev go out of scope here. 
        // Valgrind will tell us if the 'ev.on' lambda leaked.

    }

    console::log("Test Finished. Check Valgrind report.");

}