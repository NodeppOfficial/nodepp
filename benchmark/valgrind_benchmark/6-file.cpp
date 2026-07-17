#include <nodepp/nodepp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){ 

    for( int x=1000; x-->0; ){
    auto file = fs::readable("../../LICENSE");
    auto fd   = file.get_fd();

      //file.onDrain([=](){ console::log( "drained", file.get_fd() ); }); <- will cause memory leak because of circular reference.
        file.onDrain([=](){ console::log( "drained", fd ); });
        file.onClose([=](){ console::log( "closed" , fd ); }); stream::pipe( file );

    }

}