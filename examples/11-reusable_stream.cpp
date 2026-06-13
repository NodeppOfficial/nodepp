#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){
    
    file_t file ( "output.txt", "w" );

    file.set_reusable( true );
    file.onDrain([=](){ console::log( "drained" ); });
    file.onClose([=](){ console::log( "closed"  ); });

    for( auto x=3; x-->0; ){
        file_t finp ( "LICENSE", "r" );
        stream::await( finp, file );
        file.write("\n---\n");
    }

}