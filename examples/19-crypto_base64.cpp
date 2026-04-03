#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::encoder::BASE64 atob;
    atob.update( "hello" );
    atob.update( " " );
    atob.update( "workd" );

    string_t enc = atob.get();
    console::log( "->", enc );

    crypto::decoder::BASE64 btoa;
    btoa.update( enc );

    string_t dec = btoa.get();
    console::log( "->", dec );

}