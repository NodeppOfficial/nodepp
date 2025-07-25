#include <nodepp/nodepp.h>
#include <nodepp/zlib.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    auto write= fs::writable("PATH_1"); // = file_t( "PATH", "w" )
    auto read = fs::readable("PATH_2"); // = file_t( "PATH", "r" )

    read.onClose.once([](){ console::log("done"); });

    zlib::gzip::pipe( read, write );

}