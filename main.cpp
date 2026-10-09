#include <nodepp/nodepp.h>
#include <nodepp/timer.h>

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain() {

    console::log( encoder::hex::atob( 17 ) );
    console::log( encoder::hex::atob(  7 ) );
    console::log( encoder::hex::atob( 13 ) );

    timer::add([=](){
        console::log( os::rand() );
    return 1; }, 1000 );

}

/*────────────────────────────────────────────────────────────────────────────*/