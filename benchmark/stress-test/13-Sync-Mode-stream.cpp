#include <nodepp/nodepp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {

    file_t file ( "LICENSE", "r" );

    while( file.is_available() ){
        console::log( ">>", file.read_line() ); // <- get stuck here
    }

}

//bug fixed