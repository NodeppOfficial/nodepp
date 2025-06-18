#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>
#include <nodepp/crypto.h>

using namespace nodepp;

void onMain(){

    crypto::sign::DSA ppt;
    string_t msg = "Hello World!";
    auto    sign = ppt.sign( msg );
    ptr_t<uchar> data ( sign.size() );
    
    memcpy( &data, sign.get(), data.size() );

    console::log( "sign:", encoder::hex::get( data ) );

    console::log( "verf:", ppt.verify( msg, sign ) );
    console::log( "verf:", ppt.verify( msg, "Hello World!" ) );

}