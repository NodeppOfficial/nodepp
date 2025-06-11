#include <nodepp/nodepp.h>
#include <nodepp/file.h>

using namespace nodepp;

void onMain(){
    
    // Write File
    file_t fileA ( "file.txt", "w" );
    fileA.write( "Hello Worl!" );

    /*-----------------------------------------*/

    // Read File
    file_t fileB ( "file.txt", "r" );
    console::log( fileB.read() );

}