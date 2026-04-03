#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    encoder_t atob ("0123456789");
    atob.update( "hello" );
    atob.update( " " );
    atob.update( "world" );

    string_t enc = atob.get();
    console::log( "->", enc );

    decoder_t btoa ("0123456789");
    btoa.update( enc );

    string_t dec = btoa.get();
    console::log( "->", dec );

}