#include <nodepp/nodepp.h>
#include <nodepp/file.h>

using namespace nodepp;

void onMain(){
    
    do { // Write File
        
        file_t file( "file.txt", "w" );
        file.write ( "Hello Worl!" );

    } while(0);

    /*-----------------------------------------*/
   
    do { // Read File
        
        file_t file ( "file.txt", "r" );
        console::log( file.read() );

    } while(0);

}