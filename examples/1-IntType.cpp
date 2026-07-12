#include <nodepp/nodepp.h>
#include <nodepp/https.h>

using namespace nodepp;

void onMain(){

    console::log( sizeof( int_8  ) ); // << 1
    console::log( sizeof( int_16 ) ); // << 2
    console::log( sizeof( int_32 ) ); // << 4
    console::log( sizeof( int_64 ) ); // << 8

}