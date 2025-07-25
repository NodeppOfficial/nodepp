#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::encrypt::XOR XOR ( "PASSWORD" );
    crypto::encrypt::XOR ROX ( "PASSWORD" );
    string_t msg = "Hello World!";

    XOR.update( msg );
    auto enc = XOR.get();
    console::log( "->", enc );

    ROX.update( enc );
    auto dec = ROX.get();
    console::log( "->", dec );

}