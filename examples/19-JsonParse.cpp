#include <nodepp/nodepp.h>
#include <nodepp/json.h>

using namespace nodepp;

void onMain(){

    object_t ppt = json::parse(R"({
         "var1": 10,
         "var4": true,
         "var3": 10.99,
         "var2": "Hello World",
         "var6": { "var1": "Nice Work" },
         "var7": [ "Hello", 10, "World" ],
    })");

    console::log( ppt["var3"].as<float>() );
    console::log( ppt["var7"][0].as<string_t>() );
    console::log( ppt["var6"]["var1"].as<string_t>() );

}