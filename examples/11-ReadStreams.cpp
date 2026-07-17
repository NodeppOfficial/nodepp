#include <nodepp/nodepp.h>
#include <nodepp/path.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){
    
    auto file = fs::readable( "LICENSE" );
    // file.set_range( 128, 1024 );
    
    file.onData.on([]( string_t chunk ){
        console::log( chunk );
    });

    stream::pipe( file );
    
}