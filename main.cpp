#include <nodepp/nodepp.h>

using namespace nodepp;

/*────────────────────────────────────────────────────────────────────────────*/

void onMain() {

    handler_t<int> hdl;

    auto hdl1 = hdl.create(); hdl.update( hdl1, 10 );
    auto hdl2 = hdl.create(); hdl.update( hdl2, 20 );
    auto hdl3 = hdl.create(); hdl.update( hdl3, 30 );
    auto hdl4 = hdl.create(); hdl.update( hdl4, 40 );

    hdl.get_queue().swap( 
        hdl.get_queue().first(), 
        hdl.get_queue().last () 
    );

    console::log( hdl.read( hdl1 )[0] );
    console::log( hdl.read( hdl4 )[0] );

}

/*────────────────────────────────────────────────────────────────────────────*/