#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    array_t<uint> arr ({ 10, 50, 90, 20 });
    arr.unshift( 30 ); arr.push( 70 );
    arr.insert( 2, 100 );

    console::log( arr.join(" ") );

    console::log( "-- --" );
    arr = arr.sort([]( uint a, uint b ){ return a<=b; });
    console::log( arr.join(" ") );

    console::log( "-- --" );
    console::log( arr.slice( 0, 2 ).join(" ") );

    console::log( "-- --" );
    console::log( arr.slice( 3 ).join(" ") );

    console::log( "-- --" );
    console::log( arr.slice( -3 ).join(" ") );

    console::log( "-- --" );
    console::log( arr.splice( 1, 3, { 1, 2, 3 } ).join(" ") );

    console::log( "-- --" );
    console::log( arr.join(" ") );

    console::log( "-- --" );

    console::log( "is_empty:", arr.empty() ? "true" : "false" );
    console::log( "size:", arr.size() );
    console::log( "addr:", arr.data() ); arr.clear();
    console::log( "is_empty:", arr.empty() ? "true" : "false" );

}