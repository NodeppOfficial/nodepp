#include <nodepp/nodepp.h>
#include <nodepp/json.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain(){

    console::log( 
        encoder::hex::atob( os::rand() ),
        encoder::hex::atob( os::rand() ),
        encoder::hex::atob( os::rand() ),
        encoder::hex::atob( os::rand() ),
        sizeof( uchar_64 ) * 4 * 8
    );

}