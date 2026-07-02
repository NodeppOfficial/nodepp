#include <nodepp/nodepp.h>
#include <nodepp/handler.h>

using namespace nodepp;

void onMain(){

    handler_t<string_t> hdl;

    auto addr = hdl.create();
    hdl.update( addr, "hello world" );

    auto val = hdl.read( addr );
    
    if( !val.null() ){
             console::log( "->", *val );
    } else { console::log( "-> not found" ); }

}