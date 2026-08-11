#include <nodepp/nodepp.h>
#include <nodepp/event.h>
#include <nodepp/timer.h>

using namespace nodepp;

GENERATOR( process_1 ) {
    
    int counter = 10;

    coEmit(){ 
    coBegin
    
        while( counter-->0 ){
            console::done( ":>", counter ); 
            coNext;
        }

    coFinish
    }

};

void onMain(){ 

    event_t<> ev;

    ev.add    ( process_1() );
    timer::add( process_1() ,1000 ); 

}