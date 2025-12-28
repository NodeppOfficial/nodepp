#include <nodepp/nodepp.h>

using namespace nodepp;

void onMain(){

    ptr_t<int> GC   ( 0UL, 10 );              // SSO optimization and automatic heap
//  ptr_t<int> GC = new int(10);              // No SSO optimization always heap
//  ptr_t<int> GC = type::bind( (int)10 )     // SSO optimization and automatic heap
//  ptr_t<int> GC = type::bind( new int(10) ) // NEVER USE ( new ) WITH TYPE::BIND

    console::log("-- 0 --");
    console::log( "addr ->",  GC );
    console::log( "value->", *GC );
    console::log( "count->",  GC.count() );

    process::add([=](){
        console::log("-- 1 --");
        console::log( "addr ->",  GC );
        console::log( "value->", *GC );
        console::log( "count->",  GC.count() );
    return -1; });

    process::await([&](){
        console::log("-- 2 --");
        console::log( "addr ->",  GC );
        console::log( "value->", *GC );
        console::log( "count->",  GC.count() );
    return -1; });

    process::onSIGCLOSE.once([=](){
        console::log("-- 3 --");
        console::log( "addr ->",  GC );
        console::log( "value->", *GC );
        console::log( "count->",  GC.count() );
    });

    console::log("-- 4 --");
    console::log( "addr ->",  GC );
    console::log( "value->", *GC );
    console::log( "count->",  GC.count() );

}