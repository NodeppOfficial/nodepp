#include <nodepp/nodepp.h>
#include <nodepp/path.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){
    
    file_t read ( "Path/To/File1", "r" );
    file_t write( "Path/To/File2", "w" );

    read.onData.on([]( string_t chunk ){
        console::log( chunk );
    });

    read.onClose.on([](){
        console::log("closed");
    });

    stream::pipe( read, write );
    
}