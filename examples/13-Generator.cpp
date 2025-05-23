#include <nodepp/nodepp.h>

using namespace nodepp;

GENERATOR( process_1 ) {
    
    int counter = 10;

    coEmit(){ 
    coStart
    
        while( counter-->0 ){
            console::done( ":>", counter ); 
            coNext;
        }

    coStop
    }

};

void onMain(){
    process_1 A; process::add(A); 
}