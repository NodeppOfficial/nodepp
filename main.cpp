#include <nodepp/nodepp.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    auto obj = object_t();
    auto raw = string_t( "hello world!" );
    auto idx = raw.slice_view( 0, 5 );

    obj["moj"] = 20;
    obj[ idx ] = 10;

    console::log( json::stringify( obj ), json::stringify( idx ), string::to_string( idx ) );
    

}