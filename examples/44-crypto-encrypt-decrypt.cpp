#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::encrypt::AES_256_ECB atob ( "SECRET" );
    atob.update( "hello" );
    atob.update( " " );
    atob.update( "workd" );

    string_t enc = atob.get();
    console::log( "->", encoder::buffer::atob(enc) );

    crypto::decrypt::AES_256_ECB btoa ( "SECRET" );
    btoa.update( enc );

    string_t dec = btoa.get();
    console::log( "->", dec );

}