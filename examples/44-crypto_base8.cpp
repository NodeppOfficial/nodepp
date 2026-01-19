#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::encoder::BASE8 atob;
    atob.update( "hello" );
    atob.update( " " );
    atob.update( "world" );

    string_t enc = atob.get();
    console::log( "->", enc );

    crypto::decoder::BASE8 btoa;
    btoa.update( enc );

    string_t dec = btoa.get();
    console::log( "->", dec );

}