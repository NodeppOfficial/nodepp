#include <nodepp/nodepp.h>
#include <nodepp/crypto.h>

using namespace nodepp;

string_t atob( uchar_64 addres ){

    string_t tmp( sizeof(uchar_64), 0x00 );
    memcpy( tmp.get(), &addres, sizeof( uchar_64 ) );

    auto enc = crypto::encoder::BASE58();
    enc.update( string::to_string(tmp) );
    return enc.get();

}

uchar_64 btoa( string_t code ){

    auto dec = crypto::decoder::BASE58();
    uchar_64 out; dec.update( code );
    auto tmp = dec.get();

    memcpy( &out, tmp.get(), tmp.size() );
    return out;

}

void onMain(){
    
    auto z = process::invoke([=]( any_t ){
        console::log( "hello world!" );
    return -1; });

    auto a = atob( z );
    auto b = btoa( a );

    console::log( "->", z );
    console::log( "->", a );
    console::log( "->", b );

    process::call( b, nullptr );
    
}