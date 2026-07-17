#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    ptr_t<char> ptr ( 13UL, 0x00 );
    memcpy( ptr.get(), "hello world!", 12 );

    array_t<char> arr = ptr;
    string_t /**/ str = arr.ptr();

    for( auto &x: str ){ x = string::to_upper( x ); }

    console::log( "0>", arr.join( string::null() ) );
    console::log( "1>", ptr.get() );
    console::log( "2>", str );

}