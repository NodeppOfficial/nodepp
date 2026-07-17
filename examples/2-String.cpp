#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    string_t str = "adcbz"; str += "jpog";
    console::log( str );

    console::log( "-- --" );
    str = str.sort([]( char a, char b ){ return b > a; });
    console::log( str );

    console::log( "-- --" );
    console::log( str.slice( 1, 3 ) );

    console::log( "-- --" );
    console::log( str.splice( 1, 3, "abc" ) );

    console::log( "-- --" );
    console::log( str );

    console::log( "-- --" );

    console::log( "is_empty:", str.empty() ? "true" : "false" );
    console::log( "size:", str.size() );
    console::log( "addr:", str.data() ); str.clear();
    console::log( "is_empty:", str.empty() ? "true" : "false" );

}