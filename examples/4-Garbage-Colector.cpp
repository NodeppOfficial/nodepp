#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    ptr_t<uint> GC = new uint(10);

    console::log("-- --");
    console::log( "value->", *GC );
    console::log( "addr ->",  GC );
    console::log( "count->",  GC.count() );

    process::add([=](){ // <- note = and not & | this is important to correctly share the memory across tasks

        console::log("-- --");
        console::log( "value->", *GC );
        console::log( "addr ->",  GC );
        console::log( "count->",  GC.count() );

    return -1;
    });

    process::await([&](){ // <- note & instead = | but you are still able to use =

        console::log("-- --");
        console::log( "value->", *GC );
        console::log( "addr ->",  GC );
        console::log( "count->",  GC.count() );

    return -1;
    });

    console::log("-- --");
    console::log( "value->", *GC );
    console::log( "addr ->",  GC );
    console::log( "count->",  GC.count() );

}