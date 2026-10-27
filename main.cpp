#define NODEPP_ALLOW_PTR_ATOMIC_COUNTER 1

#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    ptr_t<string_t> tmp ( 0UL );
    string_t ppt = "hola mundo";
    tmp[0] = ppt;

    console::log( ">>", (void*) ppt   .get() );
    console::log( ">>", (void*) tmp[0].get() );

}