#include <nodepp.h>
#include <nodepp/map.h>

using namespace nodepp;

void onMain(){

    map_t<uchar_8,string_t> map; // <- direct binary tier address (FASTEST)

    console::log( ">>", type::is_integral<uchar_8 >::value );
    console::log( ">>", type::is_integral<uchar_16>::value );
    console::log( ">>", type::is_integral<uchar_32>::value );
    console::log( ">>", type::is_integral<uchar_64>::value );

    map[0b00000000] = "hello world 1";
    map[0b00000001] = "hello world 2";
    map[0b00000010] = "hello world 3";

    console::log( "--> a", map[0b00000000] );
    console::log( "--> b", map[0b00000001] );
    console::log( "--> c", map[0b00000010] );

}