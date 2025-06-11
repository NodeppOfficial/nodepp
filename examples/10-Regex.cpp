#include <nodepp/nodepp.h>
#include <nodepp/regex.h>

using namespace nodepp;

void onMain(){

    string_t message = R"(
        ![Imagen1](URL1)
        ![Imagen2](URL2)
        ![Imagen3](URL3)
        ![Imagen4](URL5)
    )";

    regex_t reg ("!\\[([^\\]]+)\\]\\(([^\\)]+)\\)");

    console::log( "-- --" );
    for( auto &x: reg.match_all( message ) ){
         console::log( "->", x );
    }

    console::log( "-- --" );
    for( auto &x: reg.get_memory() ){
         console::log( "->", x );
    }

}