#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::sign::DH key; key.generate_keys();

    auto skey = key.get_public_key();
    auto sign = key.sign( skey );

    console::log( sign );
    console::log( "verified", key.verify( skey, sign ) );

}