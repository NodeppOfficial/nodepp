#include <nodepp/nodepp.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    object_t ppt ({ 
        { "var", "mojon" },
        { "vor", ptr_t<string_t>({ "hello world", "mojon" }) }
        
    });

    console::log( ppt["var"].as<string_t>() );
    console::log( json::format( ppt ) );

}