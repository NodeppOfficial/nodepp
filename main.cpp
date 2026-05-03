#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    auto x = string::split( "hola,mundo,de,mierda,", "," );
    for( auto y: x ){
         console::log( ">>", y );
    }

}