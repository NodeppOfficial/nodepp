#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::hash::SHA3_256 hash;
    hash.update( "hello" );
    hash.update( " " );
    hash.update( "workd" );

    console::log( hash.get() );

}