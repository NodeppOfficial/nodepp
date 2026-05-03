#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    // Load Environment Variables using a .env file
    process::env::init(".env");
    console::log( process::env::get("VARA") );
    console::log( process::env::get("VARB") );

    // Add Environment Variables
    process::env::set( "mode", "hello" );
    console::log( process::env::get("mode") );

    // Delete Environment Variables
    process::env::remove( "mode" );
    console::log( process::env::get("mode") );

    // Add Variables using command line
    /* g++ -o main main.cpp ; ./main ?NAME_1=VALUE ?NAME_2=VALUE */
    /* eg: ./main ?mode=client */

}