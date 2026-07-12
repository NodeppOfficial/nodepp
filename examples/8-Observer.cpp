#include <nodepp/nodepp.h>
#include <nodepp/observer.h>

using namespace nodepp;

void onMain(){

    observer_t obj ({
        { "var1", "hello world!" },
        { "var2", 10   },
        { "var3", 69.f }
    });

    obj.on( "var1", [=]( observer_t self, any_t before, any_t after ){
        console::log( "var1:", after.as<string_t>() );
    });

    obj.on( "var2", [=]( observer_t self, any_t before, any_t after ){
        console::log( "var2:", after.as<int>() );
    });

    obj.on( "var3", [=]( observer_t self, any_t before, any_t after ){
        console::log( "var3:", after.as<float>() );
    });

    obj.set( "var1", "nodepp is awasome" );
    obj.set( "var2", 20 );
    obj.set( "var3", 20.f );

}