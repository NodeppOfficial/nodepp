#include <nodepp/nodepp.h>
#include <nodepp/cookie.h>

using namespace nodepp;

void onMain(){

    auto q = cookie::parse( "var1=10;var2=20;var3=30" );

    for( auto x: q.data() ){
         console::log( x.first, ":", x.second );
    }

    console::log( q["var1"] );
    console::log( q["var2"] );
    console::log( q["var3"] );

    q["var4"] = "Hello World!";

    console::log( cookie::format(q) );

}