#include <nodepp/nodepp.h>
#include <nodepp/path.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){
    
    auto _str = fs::readable( "Path/To/File" );

    _str.onData.on([]( string_t chunk ){
        console::log( chunk );
    });

    stream::pipe( _str );
    
}