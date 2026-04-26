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

    // nodepp compile regex but if you need to be faster use static to pre-compile regex and keep
    // as long as the function lives, but take care with () memory copy, because can lead 
    // to race conditions if shared with multiple threads
    // thread_local static regex_t( "(pattern)" ); <- multithread safe
    // /*--------*/ static regex_t( "(pattern)" ); <- multithread unsafe

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