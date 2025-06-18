#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::sign::DH key1, key2, key3;

    auto skey1 = key1.get_public_key();
    auto sign1 = key1.sign( skey1 );

    console::log( sign1 );

    console::log( "verified", key1.verify( skey1, sign1 ) );

}