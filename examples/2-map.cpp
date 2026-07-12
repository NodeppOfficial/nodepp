#include <nodepp/nodepp.h>
#include <nodepp/json.h>
#include <nodepp/map.h>

using namespace nodepp;

void onMain(){

    map_t<string_t,string_t> map;

    map["a"] = "hello world";
    map["b"] = "hello world";
    map["c"] = "hello world";

    console::log( "--> a", map["a"] );
    console::log( "--> b", map["b"] );
    console::log( "--> c", map["c"] );

    console::log( json::stringify(map) );
    
}