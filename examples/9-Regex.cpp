#include <nodepp/nodepp.h>
#include <nodepp/regex.h>

using namespace nodepp;

void onMain(){

    string_t message = "Hello 123 Dude";

    console::log( "-> \\d+", regex::test( message, "\\d+" ) ? "true" : "false" );

    console::log( "-> 123", regex::test( message, "123" ) ? "true" : "false" );

    console::log( "-> aaa", regex::test( message, "aaa" ) ? "true" : "false" );

    console::log( "-- --" );

    console::log( "->", regex::match( message, "[0-9]+" ) );

    console::log( "->", regex::split( message, "\\s+" ).join(" | ") );

    console::log( "->", regex::match_all( message, "[0-9a-z]+", true ).join(" | ") );

    console::log( "->", regex::replace( message, "\\d+", "world" ) );

    console::log( "->", regex::replace_all( message, "[0-9a-z]+", "nodepp", true ) );

    console::log( "-- --" );

    for( auto &x: regex::split( message, "\\s+" ) ){
         console::log( "-->", x );
    }

}